#ifndef FIFO_H
#define FIFO_H
#include <stdio.h>

typedef void * FIFO_HAND;
/***************************************************
*函数名:	fifo_InitFifo()                                          
*函数功能:	初始化队列      
*参数:		iSize 空间尺寸字节单为
*		iProctection 保存空间尺寸 
*返回值:	FIFO_HAND 队列句柄                                                           
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
FIFO_HAND fifo_InitFifo(int iSize,int iProctection);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_PushRang(FIFO_HAND hFifo,unsigned char * aArray,int iSize);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_Push(FIFO_HAND hFifo,unsigned char ch);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_IsEmpty(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_IsFull(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_Pop()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_Pop(FIFO_HAND hFifo,unsigned char * pCh);
/***************************************************
*函数名:	fifo_Read()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_Read(FIFO_HAND hFifo, unsigned char * chArray,int iNum);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_PopRang(FIFO_HAND hFifo, unsigned char * chArray,int * iNum);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_GetBlock(FIFO_HAND hFifo,int iEndFix,char * buf,int *num);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_FindStrs(FIFO_HAND hFifo,unsigned char * strs,int num);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_Remove(FIFO_HAND hFifo,int iDelNum);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_RemoveAll(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_GetEndFix(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_PopWord(FIFO_HAND hFifo,short *value);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_GetIndex(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_GetOfFromCount(FIFO_HAND hFifo,int iBegin);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_GetLength(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_IsProtection(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_Destory()                                          
*函数功能:	释放队列句柄和申请的空间
*参数:		hFifo 队列句柄
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_Destory(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_Clean(FIFO_HAND hFifo);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_ReadWord(FIFO_HAND hFifo, short int * pValue);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_Backspace(FIFO_HAND hFifo,int iNum);
/***************************************************
*函数名:	fifo_PushRang()                                          
*函数功能:	弹出iSize字节的数据      
*参数:		hFifo 队列句柄
* 		aArray 提取数据的空间
*		iSize 提取数据尺寸 
*返回值:	<0 操作失败
*		>0 读取的数据尺寸                                                         
*作者:		zhaoaq                                                                
*日期:		2015-3-8                                                         
****************************************************/
int fifo_Distance(FIFO_HAND hFifo,int iBegin,int iEnd);


int fifo_get_cache_free(FIFO_HAND hFifo);
#endif
