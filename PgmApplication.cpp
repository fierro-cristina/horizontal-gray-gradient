// PgmApplication.cpp: defines the point of exit for the console application.
//
#include "stdafx.h"
#include <clocale>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cmath>
#include <iostream>

using namespace std;

struct PGMData {
	int row;
	int col;
	int max_gray;
	int **matrix;

	PGMData() {
		row = col = 0;
		max_gray = 255;
		matrix = nullptr;
	}
};

int writePGM(const char *filename, const PGMData *data) {
	FILE *pgmFile = fopen(filename, "wb");
	if (pgmFile == NULL) {
		cout << "Error creating the file" << endl;
		return -1;
	}

	fprintf(pgmFile, "P5 ");
	fprintf(pgmFile, "%d %d ", data->col, data->row);
	fprintf(pgmFile, "%d ", data->max_gray);

	for (int i = 0; i < data->row; ++i)
		for (int j = 0; j < data->col; ++j)
			fputc(data->matrix[i][j], pgmFile);

	fflush(pgmFile);
	fclose(pgmFile);

	cout << "File " << filename << " has been successfully created." << endl;
	return 0;
}

void fillNoDither(const PGMData *data)
{
	for (int j = 0, val; j < data->col; j++)
	{
		val = data->max_gray - (j / (float)data->col) * 255;
		for (int i = 0; i < data->row; i++)
			data->matrix[i][j] = val;
	}
}

void randomDither(const PGMData *data)
{
	for (int i = 0, r; i < data->row; i++)
	{
		for (int j = 0; j < data->col; j++)
		{
			r = rand() % 256;
			if (r < data->matrix[i][j])
				data->matrix[i][j] = data->max_gray;
			else
				data->matrix[i][j] = 0;
		}
	}
}

void orderedDither(const PGMData *data)
{
	/*dither matrix*/
	unsigned int dith[4][4] = { { 1,   9,  3, 11 },
	{ 13,  5, 15,  7 },
	{ 4, 12,  2, 10 },
	{ 16,  8, 14,  6 } };
	float ratio = 1.0 / 15;

	for (int i = 0; i < data->row; i++)
	{
		for (int j = 0; j < data->col; j++)
		{
			int oldPixel = data->matrix[i][j];
			int value = (oldPixel * ratio > dith[i % 4][j % 4]) ? data->max_gray : 0;
			data->matrix[i][j] = value;
		}
	}
}

void FloydSteinbergDither(const PGMData *data)
{
	for (int i = 0; i < data->row - 1; i++)
	{
		for (int j = 1; j < data->col - 1; j++)
		{
			int P = data->matrix[i][j] > 127 ? data->max_gray : 0;
			double e = data->matrix[i][j] - P;

			data->matrix[i][j] = P;
			data->matrix[i][j + 1] += (e * 7 / 16);

			data->matrix[i + 1][j - 1] += (e * 3 / 16);
			data->matrix[i + 1][j] += (e * 5 / 16);
			data->matrix[i + 1][j + 1] += (e * 1 / 16);
		}
	}
}

void JarvisJudiceNinkeDither(const PGMData *data)
{
	for (int i = 0; i < data->row - 2; i++)
	{
		for (int j = 2; j < data->col - 2; j++)
		{
			int P = data->matrix[i][j] > 127 ? data->max_gray : 0;
			double e = data->matrix[i][j] - P;

			data->matrix[i][j] = P;
			data->matrix[i][j + 1] += (e * 7 / 48);
			data->matrix[i][j + 2] += (e * 5 / 48);

			data->matrix[i + 1][j - 2] += (e * 3 / 48);
			data->matrix[i + 1][j - 1] += (e * 5 / 48);
			data->matrix[i + 1][j] += (e * 7 / 48);
			data->matrix[i + 1][j + 1] += (e * 5 / 48);
			data->matrix[i + 1][j + 2] += (e * 3 / 48);

			data->matrix[i + 2][j - 2] += (e * 1 / 48);
			data->matrix[i + 2][j - 1] += (e * 3 / 48);
			data->matrix[i + 2][j] += (e * 5 / 48);
			data->matrix[i + 2][j + 1] += (e * 3 / 48);
			data->matrix[i + 2][j + 2] += (e * 1 / 48);
		}
	}
}

int main(int argc, _TCHAR* argv[])
{
	if (argc < 5)
	{
		cout << "Not enough arguments in the command line" << endl;
		system("Pause");
		return -1;
	}

	const char *fileName = argv[1];
	int width = atoi(argv[2]);
	int height = atoi(argv[3]);
	int type = atoi(argv[4]);

	int **data = nullptr;
	if (width > 0 && height > 0)
	{
		data = new int*[height];
		for (int i = 0; i < height; i++)
			data[i] = new int[width];
	}

	PGMData pgmData;
	pgmData.row = height;
	pgmData.col = width;
	pgmData.max_gray = 255;
	pgmData.matrix = data;
	fillNoDither(&pgmData);

	switch (type)
	{
	case 0:	// without dithering
		break;
	case 1:	// Ordered
		orderedDither(&pgmData);
		break;
	case 2:	// Random
		randomDither(&pgmData);
		break;
	case 3:	// Floyd–Steinberg
		FloydSteinbergDither(&pgmData);
		break;
	case 4:	// Jarvis, Judice, Ninke
		JarvisJudiceNinkeDither(&pgmData);
		break;
	default:
		cout << "An unknown type of dithering has been created" << endl;
		break;
	}

	writePGM(fileName, &pgmData);
	system("Pause");
	return 0;
}

