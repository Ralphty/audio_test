/***************************************************************
文件名：list_lib.h
文件说明:双向循环链表操作的基础类库
作者:crazy
日期:2014-09-05
***************************************************************/

#ifndef _LIST_LIB_H_
#define _LIST_LIB_H_

#include <stdbool.h>

typedef void* LIST_HANDLER_PTR;		//链表操作的句柄指针
typedef void* DATA_PTR;			//链表节点存储的数据块的起始地址
typedef unsigned int LIST_SIZE;		//链表中的节点数
typedef unsigned int DATA_SIZE;		//链表节点存储的数据块的大小
typedef unsigned int LIST_NODE_INDEX;	//链表的节点索引
typedef bool RETURN_VALUE;
typedef int GET_SIZE;

LIST_HANDLER_PTR LS_Init(DATA_SIZE uiStruct_size);							//链表的初始化操作

RETURN_VALUE LS_Add(LIST_HANDLER_PTR pList_handler, DATA_PTR pData);				//添加链表节点

RETURN_VALUE LS_Del_byIndex(LIST_HANDLER_PTR pList_handler, LIST_NODE_INDEX uiIndex);	//删除指定索引的链表节点

GET_SIZE LS_GetSize(LIST_HANDLER_PTR pList_handler);							//获取链表中的节点数

RETURN_VALUE LS_Move_begin(LIST_HANDLER_PTR pList_handler);						//移动迭代器到链表头部

RETURN_VALUE LS_Move_next(LIST_HANDLER_PTR pList_handler);						//向后移动迭代器

RETURN_VALUE LS_Move_prev(LIST_HANDLER_PTR pList_handler);						//向前移动迭代器

DATA_PTR LS_GetData(LIST_HANDLER_PTR pList_handler);							//获取链表中迭代器所指的节点存储数据的指针

DATA_PTR LS_GetData_byIndex(LIST_HANDLER_PTR pList_handler, LIST_NODE_INDEX uiIndex);	//获取指定索引的链表节点存储数据的指针

RETURN_VALUE LS_Del(LIST_HANDLER_PTR pList_handler);							//删除迭代器所指的链表节点

RETURN_VALUE LS_Insert_byIndex(LIST_HANDLER_PTR pList_handler, LIST_NODE_INDEX uiIndex, DATA_PTR pData);	//在指定的索引节点后插入节点

RETURN_VALUE LS_Insert(LIST_HANDLER_PTR pList_handler, DATA_PTR pData);				//在迭代器所指的节点后插入节点

RETURN_VALUE LS_Destory(LIST_HANDLER_PTR pList_handler);

#endif
