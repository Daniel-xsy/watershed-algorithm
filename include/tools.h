#ifndef TOOLS_H
#define TOOLS_H

#include<math.h>
#include<stdio.h>
#include<stdlib.h>

#define PI 3.1415926

//高斯滤波
unsigned int gauss_filter(
    unsigned char **imageData, unsigned int width, unsigned int height, unsigned char kernel_size, double sigma);
//中值滤波
unsigned int median_filter(
    unsigned char **imageData, unsigned int width, unsigned int height, unsigned char kernel_size);
//提取滑动窗口
unsigned int get_window(
    const unsigned char *imageData, int x, int y, unsigned int width, unsigned int height, unsigned int kernel_s, unsigned char** image_window);
//计算中值
unsigned char get_median(unsigned char* image_window, unsigned char kernel_s);


unsigned int gauss_filter(
    unsigned char **imageData, unsigned int width, unsigned int height, unsigned char kernel_size, double sigma)
{
    // kernel_size为奇数且不超过11
    if (kernel_size % 2 == 0 || kernel_size > 11)
    {
        printf("kernel size need to be odd number and less than 11!\n");
        return 0;
    }

    unsigned int center = kernel_size / 2;
    double *gauss_window;
    unsigned char *image_window, *temp;
    int i, j, k, cnt;
    double x2, y2;

    gauss_window = (double*)malloc(sizeof(double)*kernel_size*kernel_size);
    if (gauss_window == NULL)
    {
        printf("not enough memory!\n");
        return 0;
    }
    
    for (i = 0; i < kernel_size; i++)
    {
        x2 = pow(abs(i - center), 2);
        for (j = 0; j < kernel_size; j++)
        {
            y2 = pow(abs(j - center), 2);

            cnt = i * kernel_size + j;
            gauss_window[cnt] = exp(-(x2 + y2) / (2 * sigma * sigma)) / (2 * PI *sigma * sigma);
        }
    }
    
    // 归一化
    float sum = 0;
    for (i = 0; i < kernel_size*kernel_size; i++)
    {
        sum += gauss_window[i];  
    }
    for (i = 0; i < kernel_size*kernel_size; i++)
    {
        gauss_window[i] = gauss_window[i] / sum;
    }


    image_window = (unsigned char*)malloc(sizeof(unsigned char)*kernel_size*kernel_size);
    if (image_window == NULL)
    {
        printf("not enough memory!\n");
        return 0;
    }
    // use temp or the pixel value will change accrodingly
    temp = (unsigned char*)malloc(sizeof(unsigned char) * height * width);
    if (temp == NULL)
    {
        printf("not enough memory!\n");
        return 0;
    }
    memset(temp, 0, sizeof(temp));

    // sliding window
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            get_window(*imageData, j, i, width, height, kernel_size, &image_window);
            double res = 0;
            // convolution
            for (k = 0; k < kernel_size * kernel_size; k++)
            {
                res += (double)image_window[k] * gauss_window[k];
            }
            temp[i * height + j] = (unsigned char)res;
        }
    }

    for (i = 0; i < width * height; i++)
    {
        (*imageData)[i] = temp[i];
    }

    free(gauss_window);  
    free(image_window);
    free(temp);

    return 1;
}


unsigned int median_filter(
    unsigned char **imageData, unsigned int width, unsigned int height, unsigned char kernel_size)
{
    // kernel_size为奇数且不超过11
    if (kernel_size % 2 == 0 || kernel_size > 11)
    {
        printf("kernel size need to be odd number and less than 11!\n");
        return 0;
    }

    unsigned int center = kernel_size / 2;
    unsigned char *image_window, mid, *temp;
    int i, j, k, cnt;
    double x2, y2;

    image_window = (unsigned char*)malloc(sizeof(unsigned char)*kernel_size*kernel_size);
    if (image_window == NULL)
    {
        printf("not enough memory!\n");
        return 0;
    }
    // use temp or the pixel value will change accrodingly
    temp = (unsigned char*)malloc(sizeof(unsigned char) * height * width);
    if (temp == NULL)
    {
        printf("not enough memory!\n");
        return 0;
    }
    memset(temp, 0, sizeof(temp));

    // sliding window
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            get_window(*imageData, j, i, width, height, kernel_size, &image_window);
            mid = get_median(image_window, kernel_size);
            temp[i * height + j] = mid;
        }
    }

    for (i = 0; i < width * height; i++)
    {
        (*imageData)[i] = temp[i];
    }

    free(image_window);
    free(temp);

    return 1;
}


unsigned int get_window(
    const unsigned char *imageData, int x, int y, unsigned int width, unsigned int height, unsigned int kernel_s, unsigned char** image_window)
{
    if (*image_window == NULL)
    {
        printf("invalid image_window pointer\n");
        return 0;
    }

    int i, j, center;
    unsigned int cnt = 0;

    center = kernel_s / 2;
    for (i = y - center; i <= y + center; i++)
    {
        for (j = x - center; j <= x + center; j++)
        {
            // padding = 0
            if (i < 0 || j < 0 || i > height - 1 || j > width - 1)
            {
                (*image_window)[cnt] = 0;
            }
            else
            {
                int idx = i * height + j;
                (*image_window)[cnt] = imageData[idx];
            }

            cnt += 1;
        }
    }

    return 1;
}


unsigned char get_median(unsigned char* image_window, unsigned char kernel_s)
{
    unsigned char median, temp;
    int i,j;

    // 冒泡排序
    for (i = 0; i < kernel_s * kernel_s - 1; i++)
    {
        for (j = 1; j < kernel_s * kernel_s - i ; j++)
        {
            if (image_window[j] < image_window[j-1])
            {
                temp = image_window[j];
                image_window[j] = image_window[j-1];
                image_window[j-1] = temp;
            }
        }
    }

    median = image_window[kernel_s * kernel_s / 2 + 1];

    return median;
}

#endif