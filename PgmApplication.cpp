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

//round up to 7-bit values
void randomDither(const PGMData *data)
{
	for (int j = 0; j < data->col; j++)
	{
		float tmpPixel = (((float)j+0.5)*255.0) / data->col;
		for (int i = 0; i < data->row; i++)
		{
			//int oldPixel = data->matrix[i][j];
			//float tmpPixel = (float)oldPixel / 8;
			//float r = (float)(rand()%100)/100;
			float r = ((float)rand() - (float)rand()) / RAND_MAX;
			int tmp = (int)round(tmpPixel + r);
			if (tmp < 0)
				tmp = 0;
			if (tmp > 255)
				tmp = 255;
			data->matrix[i][j] = tmp;
			/*float f = 0.0;
			if (r < modf(tmpPixel, &f))
				data->matrix[i][j] = ceil(tmpPixel)*8;
			else
				data->matrix[i][j] = floor(tmpPixel)*8;
			if (data->matrix[i][j] >= data->max_gray)
				data->matrix[i][j] = data->max_gray;*/
		}
	}
}

void orderedDither(const PGMData *data)
{
	//dither matrix
	//7-bit: divided by eight, and round to evens 0 2 4 6 8
	unsigned int dith[4][4] = { 
		{ 0, 8, 2, 10 },
		{ 12, 4, 14, 6 },
		{ 3, 11, 1, 9 },
		{ 15, 7, 13, 5 } 
	};
	
	float ratio = 1.0/ 16 ;
	//bit reversing 
	for (int i = 0; i < data->row; i++)
	{
		for (int j = 0; j < data->col; j++)
		{
			int oldPixel = data->matrix[i][j];
			float tmpPixel = ratio * oldPixel;
			if (tmpPixel > dith[i % 4][j % 4])
				data->matrix[i][j] = ceil(tmpPixel / ratio);
			else
				data->matrix[i][j] = floor(tmpPixel / ratio);
			if (data->matrix[i][j] > data->max_gray)
				data->matrix[i][j] = data->max_gray;
			//int oldPixel = data->matrix[i][j];
			//int value = (oldPixel * ratio > dith[i % 4][j % 4]*ratio) ? data->max_gray : 0;
			//data->matrix[i][j] = value;
		}
	}
}
//the following values are normalized in floating point 
//format to 0,1 - based on pseudocode provided by Wikipedia
void FloydSteinbergDither(const PGMData *data)
{
	for (int i = 0; i < data->row - 1; i++)
	{
		for (int j = 1; j < data->col - 1; j++)
		{
			//data->matrix[i][j] is the old pixel
			int P = data->matrix[i][j] > 127 ? data->max_gray : 0;
			double e = data->matrix[i][j] - P; //quantum error

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
	int width = 1920;// atoi(argv[3]);
	int height = 800;// atoi(argv[3]);
	int type = 2;// atoi(argv[1]);

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
		cout << "Ordered dithering" << endl;
		orderedDither(&pgmData);
		break;
	case 2:	// Random
		cout << "Random dithering" << endl;
		randomDither(&pgmData);
		break;
	case 3:	// Floyd–Steinberg
		cout << "Floyd–Steinberg dithering" << endl;
		FloydSteinbergDither(&pgmData);
		break;
	case 4:	// Jarvis, Judice, Ninke
		cout << "Jarvis, Judice, Ninke dithering" << endl;
		JarvisJudiceNinkeDither(&pgmData);
		break;
	default:
		cout << "An unknown type of dithering" << endl;
		break;
	}

	writePGM(fileName, &pgmData);
	system("Pause");
	return 0;
}
