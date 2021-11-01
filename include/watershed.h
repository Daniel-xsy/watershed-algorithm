#ifndef WATERSHED_H
#define WATERSHED_H

#include"tools.h"
#include"fifo.h"

#define MASK -2
#define WSHD 0
#define INIT -1
#define INQE -3

int watershed(unsigned char **imageData, unsigned int width, unsigned int height, int levels_num);
int init_struct(const unsigned char* imageData, ElemType** pixel, unsigned int width, int total_size);
int sort_pixel(ElemType** pixel, int total_size);
int init_level(float** levels, int level_num);
int cmp(const void *a ,const void *b); // used for qsort function
int get_level_index(const ElemType* pixel, int** level_index, const float* level, int total_size);
int get_neighbours(const ElemType* pixel, ElemType** neighbours,const int* lookUpTable, int index, unsigned int height, unsigned int width);
int get_lookuptable(const unsigned char* imageData, const ElemType* pixel, int**table, unsigned int width, int total_size);

int watershed(unsigned char **imageData, unsigned int width, unsigned int height, int levels_num)
{
    QLinkList fifo;
    ElemType* pixel = NULL, *neighbours = NULL;
    ElemType temp;
    int label_center, label_neighbour;

    int flag, current_level;
    int i, j, k;
    int total_size = height * width;
    int* level_index, *lookUpTable;
    float* levels;

    init_queen(&fifo);
    printf("queen initialized\n");
    if (init_struct((*imageData), &pixel, width, total_size) == 0)
    {
        printf("init error in function `init_labels_pos`\n");
        return 0;
    }
    printf("struct initialized\n");
    if (sort_pixel(&pixel, total_size) == 0)
    {
        printf("sort error in function `sort_pixel`\n");
        return 0;
    }
    printf("pixels sorted\n");
    if(get_lookuptable(*imageData, pixel, &lookUpTable, width, total_size) == 0)
    {
        printf("construct look-up table error\n");
        return 0;
    }

    if (init_level(&levels, levels_num) == 0)
    {
        printf("init error in function `init_level`\n");
        return 0;
    }
    printf("level initilized\n");
    if (get_level_index(pixel, &level_index, levels, total_size) == 0)
    {
        printf("error in function `get_level_index`\n");
        return 0;
    }

    int stop;
    for(i = 0; i < total_size; i++)
    {
        if(level_index[i] == -1)
        {
            stop = i;
            break;
        }
    }

    int start_idx = 0;
    for(i = 0; i < stop; i++) // stop_idx = level_index[i]
    {
        printf("level %i/%i\r", i + 1, stop);

        for(j = start_idx; j < level_index[i]; j++)
        {
            // Mask all pixels at the current level.
            pixel[j].label = MASK;
            get_neighbours(pixel, &neighbours, lookUpTable,j, height, width);
            // Initialize queue with neighbours of existing basins at the current level.
            for(k = 0; k < 8; k++) // 每个像素有8个邻域
            {
                if(neighbours[k].edge != 0) //该邻居不存在(边界溢出)
                    continue;
                if(neighbours[k].label >= WSHD)
                {
                    pixel[j].label = INQE;
                    add_queen(&fifo, pixel[j]); //加入队列
                    break;
                }
            }
        }
    

        while(fifo->len != 0)
        {
            pop_queen(&fifo, &temp);
            int idx_center = temp.x + temp.y * width; // temp在pixel中的位置
            int idx_neighbour;
            int flag = 0;
            get_neighbours(pixel, &neighbours, lookUpTable, idx_center, height, width);
            for(j = 0; j < 8; j++)
            {
                if(neighbours[j].edge != 0) //溢出
                {
                    continue;
                }

                label_center = temp.label;
                label_neighbour = neighbours[j].label;

                if(label_neighbour > 0)
                {
                    if(label_center == INQE || (label_center == WSHD && flag))
                    {
                        pixel[lookUpTable[idx_center]].label = label_neighbour;
                    }
                    else if (label_center > 0 && label_center != label_neighbour)
                    {
                        pixel[lookUpTable[idx_center]].label = WSHD;
                        flag = 0;
                    }
                    
                }
                else if(label_neighbour == WSHD)
                {
                    if(label_center == INQE)
                    {
                        pixel[lookUpTable[idx_center]].label = WSHD;
                        flag = 1;
                    }
                }
                else if(label_neighbour == MASK)
                {
                    idx_neighbour = neighbours[j].x + neighbours[j].y * width;
                    pixel[lookUpTable[idx_neighbour]].label = INQE;
                    add_queen(&fifo, pixel[lookUpTable[idx_neighbour]]);
                }
            }
        }

        for(j = start_idx; j < level_index[i]; j++) // stop_idx = level_index[i]
        {
            if(pixel[j].label == MASK)
            {
                add_queen(&fifo, pixel[j]);
                pixel[j].label = 255;
                while (fifo->len != 0)
                {
                    int idx_center = pixel[j].x + pixel[j].y * width;
                    int idx_neighbour;
                    pop_queen(&fifo, &temp);
                    get_neighbours(pixel, &neighbours, lookUpTable, idx_center, height, width);
                    for(k = 0; k < 8; k++)
                    {
                        if(neighbours[k].edge != 0) //溢出位置
                        {
                            continue;
                        }
                        
                        if(neighbours[k].label == MASK)
                        {
                            add_queen(&fifo, neighbours[k]);
                            idx_neighbour = neighbours[k].x + neighbours[k].y * width;
                            pixel[lookUpTable[idx_neighbour]].label = 255;
                        }
                    }
                }
                
            }
        }

        start_idx = level_index[i];

    }

    // change pixel
    for(i = 0; i < total_size; i++)
    {
        if (pixel[lookUpTable[i]].label == 255)
        {
            (*imageData)[i] = 255;
        }
        else
        {
            (*imageData)[i] = 1;
        }
    }

    delete_queen(&fifo);
    free(pixel);
    free(levels);
    free(level_index);
    free(lookUpTable);
    free(neighbours);
    return 1;
}

int init_struct(const unsigned char* imageData, ElemType** pixel, unsigned int width, int total_size)
{
    int i;

    if(pixel == NULL)
    {
        printf("invalid input pointer!\n");
        return 0;
    }
    (*pixel) = (ElemType*)malloc(sizeof(ElemType) * total_size);
    if ((*pixel) == NULL)
    {
        printf("not enough memory!\n");
        return 0;
    }
    // init position as offset index from 0 to total_size - 1
    for(i = 0; i < total_size; i++)
    {
        (*pixel)[i].value = imageData[i];
        (*pixel)[i].label = INIT;
        (*pixel)[i].x = i % ((int)width);
        (*pixel)[i].y = (int)(i / ((int)width));
        (*pixel)[i].edge = 0;
    }
    return 1;
}

int init_level(float** levels, int level_num)
{
    int i;

    if (*levels == NULL)
    {
        printf("invalid input pointer!\n");
        return 0;
    }

    (*levels) = (float*)malloc(sizeof(float)*level_num);
    if ((*levels) == NULL)
    {
        printf("not enough memory!\n");
        return 0;
    }
    // init levels as the water depth level (delta level = 256 / level_num)
    for (i = 0; i < level_num; i++)
    {
        (*levels)[i] = (float)((i + 1) * 255.0 / level_num);
    }
    return 1;
}

int sort_pixel(ElemType** pixel, int total_size)
{
    qsort(*pixel, total_size, sizeof(ElemType), cmp);
    return 1;
}

int cmp(const void *a ,const void *b)
{
    return (*(ElemType *)a).value > (*(ElemType *)b).value ? 1 : -1;
}

int get_level_index(const ElemType* pixel, int** level_index, const float* level, int total_size)
{
    if(level_index == NULL)
    {
        printf("invalid input\n");
        return 0;
    }

    int i, cnt = 0, current_level = 0;

    (*level_index) = (int*)malloc(sizeof(int)*total_size);
    memset((*level_index), -1, total_size);

    for (i = 0; i < total_size; i++)
    {
        if (pixel[i].value > level[current_level])
        {
            while (pixel[i].value > level[current_level])
            {
                current_level += 1;
            }
            (*level_index)[cnt] = i;
            cnt += 1;
            
        }
        
    }
    (*level_index)[cnt] = total_size;
    return 1;
}

int get_neighbours(const ElemType* pixel, ElemType** neighbours, const int* lookUpTable, int index, unsigned int height, unsigned int width)
{
    ElemType center = pixel[index];

    if(neighbours == NULL)
    {
        printf("invalud input\n");
        return 0;
    }
    if((*neighbours) == NULL) // 第一次传入 分配空间
    {
        (*neighbours) = (ElemType*)malloc(sizeof(ElemType) * 8); // 每个像素点有8个邻域
        if((*neighbours) == NULL)
        {
            printf("not enough memory\n");
            return 0;
        }
    }

    int x = center.x, y = center.y;
    int i, j, cnt = 0;
    for(i = -1; i <= 1; i++)
    {
        if(x + i < 0 || x + i >= width) // 左右边缘溢出
        {
            (*neighbours)[cnt].edge = 1; // 溢出位置标识
            cnt += 1;
            continue;
        }
        for(j = -1; j <= 1; j++)
        {
            if(y + j < 0 || y + j >= height) // 上下边缘溢出
            {
                (*neighbours)[cnt].edge = 1; // 溢出位置标识
                cnt += 1;
                continue;
            }
            (*neighbours)[cnt] = pixel[lookUpTable[(x + i) + (y + j) * height]];
            cnt += 1;
        }
    }

    return 1;
}

// 由于qsort不能记录返回位置 
// 只能建立索引表
// TODO: 能否优化这一步 计算复杂度过高
// 可生成cache文件存取
int get_lookuptable(const unsigned char* imageData, const ElemType* pixel, int**table, unsigned int width, int total_size)
{
    if(imageData == NULL || pixel == NULL || table == NULL)
    {
        printf("invalid input\n");
        return 0;
    }

    *table = (int*)malloc(sizeof(int) * total_size);
    if((*table) == NULL)
    {
        printf("not enough memory\n");
        return 0;
    }

    FILE* f = fopen("cache.bin","rb");
    if(!f)
    {
        printf("no cache file\n");
        fclose(f);
    }
    else // 从内存中读取数据
    {
        if(fread(*table, sizeof(int) * total_size, 1, f) == 0)
        {
            fclose(f);
            printf("load cache file failure\n");
            return 0;
        }
        return 1;
    }

    printf("construct look-up table...(it may consume time)\n");
    int i, j;
    for(i = 0; i < total_size; i++)
    {
        printf("processing: %i/%i\r",i, total_size);
        for(j = 0; j < total_size; j++)
        {
            if(pixel[j].x + pixel[j].y * width == i)
            {
                (*table)[i] = j;
                break;
            }
        }
    }
    printf("\n");

    f = fopen("cache.bin","wb");
    if(!f)
    {
        printf("wrting cache file failure\n");
    }
    if(fwrite(*table, sizeof(int) * total_size, 1, f) == 0)
    {
        printf("wrting cache file failure\n");
    }
    fclose(f);

    return 1;
    printf("over!\n");
}

#endif

