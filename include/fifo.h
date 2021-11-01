#ifndef FIFO_H
#define FIFO_H

#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>

#define TURE 1
#define FALSE 0
#define OK 1
#define ERROR 0

typedef int Status;

// 用于构建像素结构体
#pragma pack(1)
typedef struct           //数据类型
{
	unsigned char value;  //像素值
	int label;  //标签
	int x, y;   //像素坐标
    int edge;
}ElemType;
#pragma pack()

typedef struct           //结点
{
	ElemType data;       
	struct LNode* next;  
}QNode, *QLink;      

typedef struct           //队列
{
	QLink front, rear;   //队列起始和结尾
	int len;             //长度
}Queen, *QLinkList;

Status init_queen(QLinkList* fifo);
Status add_queen(QLinkList* fifo, ElemType input);
Status pop_queen(QLinkList* fifo, ElemType* output);
Status delete_queen(QLinkList* fifo);

Status init_queen(QLinkList* fifo)
{
    (*fifo) = (QLinkList)malloc(sizeof(Queen));
    if (*fifo == NULL)
    {
        printf("not enough memory!\n");
        return OVERFLOW;
    }

    (*fifo)->front = NULL;
    (*fifo)->rear = (*fifo)->front;
    (*fifo)->len = 0;

    return OK;
}

Status add_queen(QLinkList* fifo, ElemType input)
{
    if ((*fifo) == NULL)
    {
        printf("invalid fifo pointer!\n");
        return ERROR;
    } 

    QLink temp;
    temp = (QLink)malloc(sizeof(QNode));
    if(temp == NULL)
    {
        printf("not enough memory!\n");
        return OVERFLOW;
    }
	temp->data = input;
    temp->next = NULL;

    if ((*fifo)->len == 0)
    {
        (*fifo)->front = (*fifo)->rear = temp;
    }
    else
    {
        (*fifo)->rear->next = temp;
        (*fifo)->rear = temp;
    }
    (*fifo)->len += 1;
    
    return OK;
}

Status pop_queen(QLinkList* fifo, ElemType* output)
{
    if ((*fifo) == NULL)
    {
        printf("invalid fifo pointer!\n");
        return ERROR;
    }

    if((*fifo)->len == 0)
    {
        printf("empty queen!\n");
        return ERROR;
    }

	*output = (*fifo)->front->data;
    if((*fifo)->len == 1)
    {
        free((*fifo)->front);
        (*fifo)->front = (*fifo)->rear = NULL;
        (*fifo)->len = 0;

    }
    else
    {
        QLink temp = (*fifo)->front->next;
        free((*fifo)->front);
        (*fifo)->front = temp;
        (*fifo)->len -= 1;
    }

	return OK;
}

Status delete_queen(QLinkList* fifo)
{
    if ((*fifo) == NULL)
    {
        printf("invalid fifo pointer!\n");
        return ERROR;
    }

    while((*fifo)->front != (*fifo)->rear)
    {
        QLink temp = (*fifo)->front->next;
        free((*fifo)->front);
    }
    free((*fifo)->front);
    (*fifo)->front = (*fifo)->rear = NULL;
    (*fifo)->len = 0;

	return OK;
}

#endif