#ifndef LOADBMP_H
#define LOADBMP_H

// Errors
#define LOADBMP_NO_ERROR 0

#define LOADBMP_OUT_OF_MEMORY 1

#define LOADBMP_FILE_NOT_FOUND 2
#define LOADBMP_FILE_OPERATION 3

#define LOADBMP_INVALID_FILE_FORMAT 4

#define LOADBMP_INVALID_SIGNATURE 5
#define LOADBMP_INVALID_BITS_PER_PIXEL 6

// Components
#define LOADBMP_RGB 3

// struct
#pragma pack(1)//单字节对齐
typedef struct tagBITMAPFILEHEADER
{
    unsigned char  bfType[2];     //文件格式
    unsigned int   bfSize;        // 文件大小 以字节为单位(2-5字节)
    unsigned short bfReserved1;   // 保留，必须设置为0 (6-7字节)
    unsigned short bfReserved2;   // 保留，必须设置为0 (8-9字节)
    unsigned int   bfOffBits;     // 从文件头到像素数据的偏移  (10-13字节)
}BITMAPFILEHEADER;
#pragma pack()

/*位图信息头*/
#pragma pack(1)
typedef struct tagBITMAPINFOHEADER
{
    unsigned int    biSize;          // 此结构体的大小 (14-17字节)
    long            biWidth;         // 图像的宽  (18-21字节)
    long            biHeight;        // 图像的高  (22-25字节)
    unsigned short  biPlanes;        // 表示bmp图片的平面属，显然显示器只有一个平面，所以恒等于1 (26-27字节)
    unsigned short  biBitCount;      // 一像素所占的位数，(28-29字节)当biBitCount=24时，该BMP图像就是24Bit真彩图，没有调色板项。
    unsigned int    biCompression;   // 说明图象数据压缩的类型，0为不压缩。 (30-33字节)
    unsigned int    biSizeImage;     // 像素数据所占大小, 这个值应该等于上面文件头结构中bfSize-bfOffBits (34-37字节)
    long            biXPelsPerMeter; // 说明水平分辨率，用象素/米表示。一般为0 (38-41字节)
    long            biYPelsPerMeter; // 说明垂直分辨率，用象素/米表示。一般为0 (42-45字节)
    unsigned int    biClrUsed;       // 说明位图实际使用的彩色表中的颜色索引数（设为0的话，则说明使用所有调色板项）。 (46-49字节)
    unsigned int    biClrImportant;  // 说明对图象显示有重要影响的颜色索引的数目，如果是0，表示都重要。(50-53字节)
}BITMAPINFOHEADER;
#pragma pack()

/*调色板结构 仅灰度图有*/
#pragma pack(1)
typedef struct tagRGBQUAD
{
    unsigned char rgbBlue;   //该颜色的蓝色分量  (值范围为0-255)
    unsigned char rgbGreen;  //该颜色的绿色分量  (值范围为0-255)
    unsigned char rgbRed;    //该颜色的红色分量  (值范围为0-255)
    unsigned char rgbReserved;// 保留，必须为0
}RGBQUAD;
#pragma pack()

unsigned int loadbmp_decode_file(
	const char *filename, unsigned char **imageData, unsigned int *width, unsigned int *height, unsigned int components);

unsigned int loadbmp_encode_file(
	const char *filename, const unsigned char *imageData, unsigned int width, unsigned int height, unsigned int components);

unsigned int loadgreybmp_decode_file(
	const char *filename, unsigned char **imageData, unsigned int *width, unsigned int *height);

unsigned int loadgreybmp_encode_file(
	const char *filename, const unsigned char *imageData, unsigned int width, unsigned int height);

#include <stdlib.h> /* malloc(), free() */
#include <string.h> /* memset(), memcpy() */
#include <stdio.h> /* fopen(), fwrite(), fread(), fclose() */

unsigned int loadbmp_decode_file(
	const char *filename, unsigned char **imageData, unsigned int *width, unsigned int *height, unsigned int components)
{
	FILE *f = fopen(filename, "rb");

	if (!f)
		return LOADBMP_FILE_NOT_FOUND;

    BITMAPFILEHEADER *fileHeader;
    BITMAPINFOHEADER *infoHeader;
    unsigned char bmp_pad[3];
    RGBQUAD *ipRGB;

	unsigned int w, h;
	unsigned char *data = NULL;

	unsigned int x, y, i, padding;

    fileHeader = (BITMAPFILEHEADER *)malloc(sizeof(BITMAPFILEHEADER));
    infoHeader = (BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));

	if (fread(fileHeader, sizeof(BITMAPFILEHEADER), 1, f) == 0)
	{
		fclose(f);
		return LOADBMP_INVALID_FILE_FORMAT;
	}

	if (fread(infoHeader, sizeof(BITMAPINFOHEADER), 1, f) == 0)
	{
		fclose(f);
		return LOADBMP_INVALID_FILE_FORMAT;
	}

	if ((fileHeader->bfType[0] != 'B') || (fileHeader->bfType[1] != 'M'))
	{
		fclose(f);
		return LOADBMP_INVALID_SIGNATURE;
	}

	if ((infoHeader->biBitCount != 24) && (infoHeader->biBitCount != 32))
	{
		printf("infoHeader->biBitCount %i",infoHeader->biBitCount);
		fclose(f);
		return LOADBMP_INVALID_BITS_PER_PIXEL;
	}

	w = infoHeader->biWidth;
	h = infoHeader->biHeight;

	if ((w > 0) && (h > 0))
	{
		data = (unsigned char*)malloc(w * h * components);

		if (!data)
		{
			fclose(f);
			return LOADBMP_OUT_OF_MEMORY;
		}

		for (y = (h - 1); y != -1; y--)
		{
			for (x = 0; x < w; x++)
			{
				i = (x + y * w) * components;

				if (fread(data + i, 3, 1, f) == 0)
				{
					free(data);

					fclose(f);
					return LOADBMP_INVALID_FILE_FORMAT;
				}

				data[i] ^= data[i + 2] ^= data[i] ^= data[i + 2]; // BGR -> RGB

			}

			padding = ((4 - (w * 3) % 4) % 4);

			if (fread(bmp_pad, 1, padding, f) != padding)
			{
				free(data);

				fclose(f);
				return LOADBMP_INVALID_FILE_FORMAT;
			}
		}
	}

	(*width) = w;
	(*height) = h;
	(*imageData) = data;

	fclose(f);

	return LOADBMP_NO_ERROR;
}

unsigned int loadbmp_encode_file(
	const char *filename, const unsigned char *imageData, unsigned int width, unsigned int height, unsigned int components)
{
	FILE *f = fopen(filename, "wb");

	if (!f)
		return LOADBMP_FILE_OPERATION;

	unsigned char bmp_file_header[14] = { 'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0 };
	unsigned char bmp_info_header[40] = { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0 };
	const unsigned char bmp_pad[3] = { 0, 0, 0 };

	const unsigned int size = 54 + width * height * 3; // 3 as the BMP format uses 3 channels (red, green, blue and NO alpha)

	unsigned int x, y, i, padding;

	unsigned char pixel[3];

	bmp_file_header[2] = (unsigned char)(size);
	bmp_file_header[3] = (unsigned char)(size >> 8);
	bmp_file_header[4] = (unsigned char)(size >> 16);
	bmp_file_header[5] = (unsigned char)(size >> 24);

	bmp_info_header[4] = (unsigned char)(width);
	bmp_info_header[5] = (unsigned char)(width >> 8);
	bmp_info_header[6] = (unsigned char)(width >> 16);
	bmp_info_header[7] = (unsigned char)(width >> 24);

	bmp_info_header[8] = (unsigned char)(height);
	bmp_info_header[9] = (unsigned char)(height >> 8);
	bmp_info_header[10] = (unsigned char)(height >> 16);
	bmp_info_header[11] = (unsigned char)(height >> 24);

	if (fwrite(bmp_file_header, 14, 1, f) == 0)
	{
		fclose(f);
		return LOADBMP_FILE_OPERATION;
	}

	if (fwrite(bmp_info_header, 40, 1, f) == 0)
	{
		fclose(f);
		return LOADBMP_FILE_OPERATION;
	}

	for (y = (height - 1); y != -1; y--)
	{
		for (x = 0; x < width; x++)
		{
			i = (x + y * width) * components;

			memcpy(pixel, imageData + i, sizeof(pixel));

			pixel[0] ^= pixel[2] ^= pixel[0] ^= pixel[2]; // RGB -> BGR

			if (fwrite(pixel, sizeof(pixel), 1, f) == 0)
			{
				fclose(f);
				return LOADBMP_FILE_OPERATION;
			}
		}

		padding = ((4 - (width * 3) % 4) % 4);

		if (fwrite(bmp_pad, 1, padding, f) != padding)
		{
			fclose(f);
			return LOADBMP_FILE_OPERATION;
		}
	}

	fclose(f);

	return LOADBMP_NO_ERROR;
}

unsigned int loadgreybmp_decode_file(
	const char *filename, unsigned char **imageData, unsigned int *width, unsigned int *height)
{

	FILE *f = fopen(filename, "rb");

	if (!f)
		return LOADBMP_FILE_NOT_FOUND;

    BITMAPFILEHEADER *fileHeader;
    BITMAPINFOHEADER *infoHeader;
	RGBQUAD *ipRGB;
    unsigned char bmp_pad[3];

	unsigned int w, h;
	unsigned char *data = NULL;

	unsigned int x, y, i, padding;

    fileHeader = (BITMAPFILEHEADER *)malloc(sizeof(BITMAPFILEHEADER));
    infoHeader = (BITMAPINFOHEADER *)malloc(sizeof(BITMAPINFOHEADER));
	ipRGB = (RGBQUAD*)malloc(sizeof(RGBQUAD));

	// 读取文件头
	if (fread(fileHeader, sizeof(BITMAPFILEHEADER), 1, f) == 0)
	{
		fclose(f);
		return LOADBMP_INVALID_FILE_FORMAT;
	}

	// 读取文件信息头
	if (fread(infoHeader, sizeof(BITMAPINFOHEADER), 1, f) == 0)
	{
		fclose(f);
		return LOADBMP_INVALID_FILE_FORMAT;
	}

	// 读取调色板数据
	for(i = 0; i<256; i++)
	{
		if (fread(ipRGB, sizeof(RGBQUAD), 1, f) == 0)
		{
			fclose(f);
			return LOADBMP_INVALID_FILE_FORMAT;
		}

	if ((ipRGB->rgbBlue != ipRGB->rgbGreen) && (ipRGB->rgbBlue!= ipRGB->rgbRed))
	{
		fclose(f);
		return LOADBMP_INVALID_SIGNATURE;
	}
	}

	if ((fileHeader->bfType[0] != 'B') || (fileHeader->bfType[1] != 'M'))
	{
		fclose(f);
		return LOADBMP_INVALID_SIGNATURE;
	}

	if (infoHeader->biBitCount != 8)
	{
		printf("infoHeader->biBitCount %i",infoHeader->biBitCount);
		fclose(f);
		return LOADBMP_INVALID_BITS_PER_PIXEL;
	}

	w = infoHeader->biWidth;
	h = infoHeader->biHeight;

	if ((w > 0) && (h > 0))
	{
		data = (unsigned char*)malloc(w * h);

		if (!data)
		{
			fclose(f);
			return LOADBMP_OUT_OF_MEMORY;
		}

		for (y = h-1; y != -1; y--)
		{
			for (x = 0; x < w; x++)
			{
				i = x + y * w;

				if (fread(data + i, 1, 1, f) == 0)
				{
					free(data);

					fclose(f);
					return LOADBMP_INVALID_FILE_FORMAT;
				}
			}
		}
	}

	(*width) = w;
	(*height) = h;
	(*imageData) = data;

	fclose(f);
	free(fileHeader);
	free(infoHeader);
	free(ipRGB);

	return LOADBMP_NO_ERROR;
}

unsigned int loadgreybmp_encode_file(
	const char *filename, const unsigned char *imageData, unsigned int width, unsigned int height)
{
	FILE *f = fopen(filename, "wb");

	if (!f)
		return LOADBMP_FILE_OPERATION;

	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	RGBQUAD* ipRGB = NULL;
	unsigned int x, y, i;

	memset(&fileHeader, 0, sizeof(BITMAPFILEHEADER));
	memset(&infoHeader, 0, sizeof(BITMAPINFOHEADER));

	fileHeader.bfType[0] = 'B',fileHeader.bfType[1] = 'M';
	fileHeader.bfOffBits = 1078;
	infoHeader.biSize = 40;
	infoHeader.biWidth = width;
	infoHeader.biHeight = height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 8;

	ipRGB = (RGBQUAD *)malloc(256 * sizeof(RGBQUAD));
	for (i = 0; i < 256; i++)
	{
		ipRGB[i].rgbBlue = ipRGB[i].rgbGreen = ipRGB[i].rgbRed = i;
	}

	if (fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, f) == 0)
	{
		fclose(f);
		return LOADBMP_FILE_OPERATION;
	}

	if (fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, f) == 0)
	{
		fclose(f);
		return LOADBMP_FILE_OPERATION;
	}
	
	if (fwrite(ipRGB, sizeof(RGBQUAD), 256, f) == 0)
	{
		fclose(f);
		return LOADBMP_FILE_OPERATION;
	}

	for (y = (height - 1); y != -1; y--)
	{
		for (x = 0; x < width; x++)
		{
			i = (x + y * width);

			if (fwrite(&imageData[i], 1, 1, f) == 0)
			{
				fclose(f);
				return LOADBMP_FILE_OPERATION;
			}

		}
	}
	fclose(f);
	free(ipRGB);
	return LOADBMP_NO_ERROR;
}

#endif