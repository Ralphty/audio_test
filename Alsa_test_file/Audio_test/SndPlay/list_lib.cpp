#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "list_lib.h"

#define FILE_PTR stdout

/*定义双向循环链表的结构体*/
struct _LIST_NODE
{
        DATA_PTR pData;                 //指向存储数据的指针
        struct _LIST_NODE *pNext;	//指向下一个链表节点的指针
        struct _LIST_NODE *pPrev;	//指向前一个链表节点的指针
};

typedef struct _LIST_NODE LIST_NODE;

/*定义对链表进行操作的句柄*/
typedef struct _LIST_HANDLER
{
        LIST_SIZE uiList_size;          //链表中的节点数
        DATA_SIZE uiData_size;          //链表中每个节点的数据块大小
        LIST_NODE *pListHead;           //链表的头节点
        LIST_NODE *pListIterator;       //遍历链表的迭代器
    pthread_mutex_t list_mutex;	//保护链表的互斥锁
}LIST_HANDLER;

/****************************************************************
函数名：LS_Init()
函数功能:双向循环链表的初始化操作
参数说明:in-DATA_SIZE uiStruct_size:链表存储的数据大小
返回值:LIST_HANDLER_PTR:链表句柄
作者:crazy
日期：2014-09-04
 ****************************************************************/
LIST_HANDLER_PTR LS_Init(DATA_SIZE uiStruct_size)
{
    int ret = 0;

    if(0 == uiStruct_size)
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return NULL;
    }

    LIST_HANDLER *pList_Handler = (LIST_HANDLER*)malloc(sizeof(LIST_HANDLER));	//初始化链表句柄

    if(NULL != pList_Handler)
    {
        pList_Handler->uiList_size = 0;
        pList_Handler->uiData_size = uiStruct_size;	//传入链表节点存储数据的大小

        pList_Handler->pListHead = (LIST_NODE *)malloc(sizeof(LIST_NODE));

        if(NULL != pList_Handler->pListHead)
        {
            (pList_Handler->pListHead)->pNext = pList_Handler->pListHead;	//初始化链表的头节点的指针指向头节点自身
            (pList_Handler->pListHead)->pPrev = pList_Handler->pListHead;

            pList_Handler->pListIterator = pList_Handler->pListHead;	//初始化迭代器指向头节点
        }
        else
        {
            fprintf(FILE_PTR, "malloc list node head failed\n");
            return NULL;
        }

        ret = pthread_mutex_init(&pList_Handler->list_mutex, NULL);		//初始化保护链表的互斥锁
        if(0 != ret)
        {
            fprintf(FILE_PTR, "mutex initialization failed\n");
        }

        return (LIST_HANDLER_PTR)pList_Handler;
    }
    else
    {
        fprintf(FILE_PTR, "malloc list handler failed\n");
        return NULL;
    }
}

/**************************************************************************
 函数名：LS_Add()
 函数功能：添加链表节点
 参数说明：in-LIST_HANDLER_PTR pList_handler:链表句柄，DATA_PTR:要存储的数据的起始地址
 返回值:RETURN_VALUE:true-添加成功，false-添加失败
 作者:crazy
 日期:2014-09-04
 *************************************************************************/
RETURN_VALUE LS_Add(LIST_HANDLER_PTR pList_handler, DATA_PTR pData)
{
    if((NULL != pList_handler) && (NULL != pData))
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        LIST_NODE *pList_Node = (LIST_NODE *)malloc(sizeof(LIST_NODE));	//初始化链表节点
        if(NULL != pList_Node)
        {
            pthread_mutex_lock(&pHandler->list_mutex);

            pList_Node->pData = (DATA_PTR)malloc(pHandler->uiData_size);	//初始化链表节点存储数据的空间
            if(NULL == pList_Node->pData)
            {
                pthread_mutex_unlock(&pHandler->list_mutex);

                fprintf(FILE_PTR, "malloc data room failed\n");
                free(pList_Node);
                return false;
            }
            else
            {
                memcpy(pList_Node->pData, pData, pHandler->uiData_size); //将指定数据存储到链表节点中
            }

            pList_Node->pNext = (pHandler->pListHead)->pNext;	//添加节点到双向循环链表
            pList_Node->pPrev = pHandler->pListHead;
            ((pHandler->pListHead)->pNext)->pPrev = pList_Node;
            (pHandler->pListHead)->pNext = pList_Node;

            (pHandler->uiList_size)++;	//链表中的节点数加1

            pthread_mutex_unlock(&pHandler->list_mutex);

            return true;

        }
        else
        {
            fprintf(FILE_PTR, "malloc list node failed\n");
            return false;
        }
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return false;
    }
}

/******************************************************************************
 函数名:LS_Del_byIndex()
 函数功能:删除指定索引的链表节点
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄，LIST_NODE_INDEX uiIndex:链表索引
 返回值:RETURN_VALUE:true-删除成功,false-删除失败
 作者:crazy
 日期:2014-09-04
 *****************************************************************************/
RETURN_VALUE LS_Del_byIndex(LIST_HANDLER_PTR pList_handler, LIST_NODE_INDEX uiIndex)
{
    LIST_NODE_INDEX uiCurrent_index = 0;
    LIST_NODE *pCurrent_node = NULL;

    if((NULL != pList_handler) && (0 != uiIndex))
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

        pCurrent_node = pHandler->pListHead;
        while(pCurrent_node->pNext != pHandler->pListHead)	//遍历双向循环链表
        {
            if(uiCurrent_index != uiIndex)			//判断当前索引是否为指定索引
            {
                pCurrent_node = pCurrent_node->pNext;
                uiCurrent_index++;
                continue;
            }
            else
            {
                (pCurrent_node->pPrev)->pNext = pCurrent_node->pNext;	//删除当前链表节点
                (pCurrent_node->pNext)->pPrev = pCurrent_node->pPrev;

                free(pCurrent_node);
                pHandler->uiList_size--;

                pthread_mutex_unlock(&pHandler->list_mutex);

                return true;
            }
        }

        pthread_mutex_unlock(&pHandler->list_mutex);

        fprintf(FILE_PTR, "the list node of the index does not exist\n");
        return false;
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return false;
    }

}

/****************************************************************************
 函数名:LS_GetSize()
 函数功能:获取链表中的节点数
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄
 返回值:GET_SIZE:链表节点数，-1-获取失败
 作者:crazy
 日期:2014-09-04
 ***************************************************************************/
GET_SIZE  LS_GetSize(LIST_HANDLER_PTR pList_handler)
{
    if(NULL != pList_handler)
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;
        return pHandler->uiList_size;	//返回链表中的节点数
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return -1;
    }
}

/*****************************************************************************
 函数名:LS_Move_begin()
 函数功能:移动迭代器到链表头部
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄
 返回值:RETURN_VALUE:true-移动成功，false-移动失败
 作者:crazy
 日期:2014-09-04
 ****************************************************************************/
RETURN_VALUE LS_Move_begin(LIST_HANDLER_PTR pList_handler)
{
    if(NULL != pList_handler)
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

        pHandler->pListIterator = pHandler->pListHead;	//移动迭代器到链表头结点

        pthread_mutex_unlock(&pHandler->list_mutex);

        return true;
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return false;
    }
}

/*******************************************************************************
 函数名:LS_Move_next()
 函数功能:向后移动迭代器
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄
 返回值:RETURN_VALUE:true-移动成功，false-移动失败
 作者:crazy
 日期:2014-09-04
 ******************************************************************************/
RETURN_VALUE LS_Move_next(LIST_HANDLER_PTR pList_handler)
{
    if(NULL != pList_handler)
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

        if((pHandler->pListIterator)->pNext != pHandler->pListHead)
        {
            pHandler->pListIterator = (pHandler->pListIterator)->pNext;	//向后移动迭代器

            pthread_mutex_unlock(&pHandler->list_mutex);

            return true;
        }
        else
        {
            pthread_mutex_unlock(&pHandler->list_mutex);

            return false;
        }
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return false;
    }
}

/*********************************************************************
 函数名:LS_Move_prev()
 函数功能:向前移动迭代器
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄
 返回值:RETURN_VALUE:true-移动成功,false-移动失败
 作者:crazy
 日期:2014-09-04
 ********************************************************************/
RETURN_VALUE LS_Move_prev(LIST_HANDLER_PTR pList_handler)
{
    if(NULL != pList_handler)
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

        if((pHandler->pListIterator)->pPrev != pHandler->pListHead)
        {
            pHandler->pListIterator = (pHandler->pListIterator)->pPrev;	//向前移动迭代器

            pthread_mutex_unlock(&pHandler->list_mutex);

            return true;
        }
        else
        {
            pthread_mutex_unlock(&pHandler->list_mutex);

            fprintf(FILE_PTR, "the list iterator is in the begin of the list\n");
            return false;
        }
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return false;
    }
}

/*************************************************************************
 函数名:LS_GetData()
 函数功能:获取迭代器所指节点存储数据的指针
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄
 返回值:DATA_PTR:存储数据的指针
 作者:crazy
 日期:2014-09-04
 ************************************************************************/
DATA_PTR LS_GetData(LIST_HANDLER_PTR pList_handler)
{
    if(NULL != pList_handler)
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

        if((NULL != pHandler->pListIterator) && (pHandler->pListIterator != pHandler->pListHead))
        {
            pthread_mutex_unlock(&pHandler->list_mutex);

            return (pHandler->pListIterator)->pData;	//返回迭代器指向的数据指针
        }
        else
        {
            pthread_mutex_unlock(&pHandler->list_mutex);

            fprintf(FILE_PTR, "get data failed\n");
            return NULL;
        }
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return NULL;
    }
}

/******************************************************************************
 函数名:LS_GetData_byIndex()
 函数功能:获取指定索引的链表节点存储数据的指针
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄，LIST_NODE_INDEX uiIndex:链表索引
 返回值:DATA_PTR:指定索引的链表节点存储数据的指针
 作者:crazy
 日期:2014-09-04
 *****************************************************************************/
DATA_PTR LS_GetData_byIndex(LIST_HANDLER_PTR pList_handler, LIST_NODE_INDEX uiIndex)
{
    LIST_NODE_INDEX uiCurrent_index = 0;
    LIST_NODE *pCurrent_node = NULL;
    if((NULL != pList_handler) && (0 != uiIndex))
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

        pCurrent_node = pHandler->pListHead;

        pCurrent_node = pCurrent_node->pNext;
        uiCurrent_index++;

        while(pCurrent_node != pHandler->pListHead)
        {
            if(uiCurrent_index != uiIndex)
            {
                pCurrent_node = pCurrent_node->pNext;
                uiCurrent_index++;
                continue;
            }
            else
            {
                pthread_mutex_unlock(&pHandler->list_mutex);

                return pCurrent_node->pData;
            }
        }

        pthread_mutex_unlock(&pHandler->list_mutex);
        fprintf(FILE_PTR, "the list node of the index does not exit\n");
        return NULL;
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return NULL;
    }
}

/**********************************************************************************
 函数名:LS_Del()
 函数功能:删除迭代器所指的链表节点
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄
 返回值:RETURN_VALUE:true-删除成功,false-删除失败
 作者:crazy
 日期:2014-09-04
 *********************************************************************************/
RETURN_VALUE LS_Del(LIST_HANDLER_PTR pList_handler)
{
    if(NULL != pList_handler)
    {
        LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

        if((NULL != pHandler->pListIterator) && (pHandler->pListIterator != pHandler->pListHead))
        {
            ((pHandler->pListIterator)->pPrev)->pNext = (pHandler->pListIterator)->pNext;   //删除当前链表节点
                        ((pHandler->pListIterator)->pNext)->pPrev = (pHandler->pListIterator)->pPrev;

                        free(pHandler->pListIterator);
                        pHandler->uiList_size--;

            pthread_mutex_unlock(&pHandler->list_mutex);

            return true;
        }
        else
        {
            pthread_mutex_unlock(&pHandler->list_mutex);

            fprintf(FILE_PTR, "delete list node failed\n");
            return false;
        }
    }
    else
    {
        fprintf(FILE_PTR, "the input parameter is invalid\n");
        return false;
    }
}

/**************************************************************************************
 函数名:LS_Insert_byIndex()
 函数功能:在指定的索引节点后插入节点
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄，LIST_NODE_INDEX uiIndex:链表索引，DATA_PTR pData:要插入的数据的起始地址
 返回值:RETURN_VALUE:true-插入成功，false-插入失败
 作者:crazy
 日期:2014-09-04
 *************************************************************************************/
RETURN_VALUE LS_Insert_byIndex(LIST_HANDLER_PTR pList_handler, LIST_NODE_INDEX uiIndex, DATA_PTR pData)
{
    LIST_NODE_INDEX uiCurrent_index = 0;
        LIST_NODE *pCurrent_node = NULL;
    LIST_NODE *pList_Node = NULL;

        if((NULL != pList_handler) && (NULL != pData))
        {
                LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

                pCurrent_node = pHandler->pListHead;

        if(0 == uiIndex)
        {
            pList_Node = (LIST_NODE *)malloc(sizeof(LIST_NODE));
                            if(NULL != pList_Node)
                                {
                                        pList_Node->pData = (DATA_PTR)malloc(pHandler->uiData_size);
                                        if(NULL == pList_Node->pData)
                                        {
                        pthread_mutex_unlock(&pHandler->list_mutex);

                                                fprintf(FILE_PTR, "malloc data room failed\n");
                                                free(pList_Node);
                                                return false;
                                        }
                                        else
                                        {
                                                memcpy(pList_Node->pData, pData, pHandler->uiData_size);
                                        }
                                }

                                pList_Node->pNext = pCurrent_node->pNext;
                                pList_Node->pPrev = pCurrent_node;
                                (pCurrent_node->pNext)->pPrev = pList_Node;
                                pCurrent_node->pNext = pList_Node;

                                pHandler->uiList_size++;

                pthread_mutex_unlock(&pHandler->list_mutex);
                                return true;

        }
        else
        {
            pCurrent_node = pCurrent_node->pNext;
            uiCurrent_index++;

                    while(pCurrent_node != pHandler->pListHead)
                    {
                            if(uiCurrent_index != uiIndex)
                            {
                                    pCurrent_node = pCurrent_node->pNext;
                                    uiCurrent_index++;
                                    continue;
                            }
                            else
                            {
                    pList_Node = (LIST_NODE *)malloc(sizeof(LIST_NODE));
                    if(NULL != pList_Node)
                    {
                        pList_Node->pData = (DATA_PTR)malloc(pHandler->uiData_size);
                        if(NULL == pList_Node->pData)
                        {
                            pthread_mutex_unlock(&pHandler->list_mutex);

                            fprintf(FILE_PTR, "malloc data room failed\n");
                            free(pList_Node);
                            return false;
                        }
                        else
                        {
                            memcpy(pList_Node->pData, pData, pHandler->uiData_size);
                        }
                    }

                    pList_Node->pNext = pCurrent_node->pNext;
                    pList_Node->pPrev = pCurrent_node;
                    (pCurrent_node->pNext)->pPrev = pList_Node;
                    pCurrent_node->pNext = pList_Node;

                                    pHandler->uiList_size++;

                    pthread_mutex_unlock(&pHandler->list_mutex);

                                    return true;
                            }
                    }
        }

                fprintf(FILE_PTR, "the list node of the index does not exist\n");
                return false;
        }
        else
        {
                fprintf(FILE_PTR, "the input parameter is invalid\n");
                return false;
        }

}

/***************************************************************************
 函数名:LS_Insert()
 函数功能:在迭代器所指的节点后插入节点
 参数说明:in-LIST_HANDLER_PTR pList_handler:链表句柄，DATA_PTR pData:要插入的数据的起始地址
 返回值:RETURN_VALUE:true-插入成功，false-插入失败
 作者:crazy
 日期:2014-09-04
 **************************************************************************/
RETURN_VALUE LS_Insert(LIST_HANDLER_PTR pList_handler, DATA_PTR pData)
{
    if((NULL != pList_handler) && (NULL != pData))
        {
                LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

        pthread_mutex_lock(&pHandler->list_mutex);

                LIST_NODE *pList_Node = (LIST_NODE *)malloc(sizeof(LIST_NODE));
                if(NULL != pList_Node)
                {
                        pList_Node->pData = (DATA_PTR)malloc(pHandler->uiData_size);
                        if(NULL == pList_Node->pData)
                        {
                pthread_mutex_unlock(&pHandler->list_mutex);

                                fprintf(FILE_PTR, "malloc data room failed\n");
                                free(pList_Node);
                                return false;
                        }
                        else
                        {
                                memcpy(pList_Node->pData, pData, pHandler->uiData_size);
                        }

                        pList_Node->pNext = (pHandler->pListIterator)->pNext;       //添加节点到双向循环链表
                        pList_Node->pPrev = pHandler->pListIterator;
                        ((pHandler->pListIterator)->pNext)->pPrev = pList_Node;
                        (pHandler->pListIterator)->pNext = pList_Node;

                        (pHandler->uiList_size)++;      //链表中的节点数加1

            pthread_mutex_unlock(&pHandler->list_mutex);
                        return true;

                }
                else
                {
            pthread_mutex_unlock(&pHandler->list_mutex);

                        fprintf(FILE_PTR, "malloc list node failed\n");
                        return false;
                }
        }
        else
        {
                fprintf(FILE_PTR, "the input parameter is invalid\n");
                return false;
        }

}

RETURN_VALUE LS_Destory(LIST_HANDLER_PTR pList_handler)
{
    int i = 0;

    if(pList_handler == NULL)
        return false;

    LIST_HANDLER *pHandler = (LIST_HANDLER *)pList_handler;

    int iSize = LS_GetSize(pList_handler);

    for(i = 0 ;i< iSize ;i++)
    {
        LS_Del_byIndex(pList_handler,0);
    }

    free(pHandler);
    pList_handler = NULL;

    return true;

}


