#include "fifo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

struct fifo_stru
{
    unsigned char * 	pCache ;					// 内存块指针
    pthread_mutex_t  	pMutex;					// 内存互斥锁
    int 			iProtectionSize;				// 内存保护区
    int 			iCacheSize ;				// 内存尺寸
    int 			iPosFront ;					// 内存头位置指针
    int 			iPosBack ;					// 内存尾位置指针
};
#define ERROR_NULL_HANDLE  -1
#define ERROR_MEM -2
#define ERROR_PARAM -3
#define ERROR_FIFO_FULL -4
#define ERROR_FIFO_EMPTY -5
#define ERROR_NOT_ENOUGH -6
/***************************************************
*函数名:	fifo_InitFifo()
*函数功能:	初始化队列
*参数:		iSize 空间尺寸字节单为
*		iProctection 保存空间尺寸
*返回值:	FIFO_HAND 队列句柄
*作者:		zhaoaq
*日期:		2015-3-8
****************************************************/

static int fifo_copy_to(struct fifo_stru* stru,unsigned char * buf,int len);
static int fifo_copy_from(struct fifo_stru* stru,unsigned char * buf,int len);

FIFO_HAND fifo_InitFifo(int iSize,int iProctection)
{
    struct fifo_stru * pfifo = NULL;		//声明结构指针
    // 参数检查
    if(iSize <=0 || iProctection <0 ||iSize <= iProctection)
        return  (FIFO_HAND)ERROR_PARAM;
    iSize +=1;
    // 分配队列内存
    pfifo = (struct fifo_stru*) malloc(sizeof(struct fifo_stru));
    // 检查空间是否分配成功
    if(pfifo == NULL) return  (FIFO_HAND)ERROR_MEM;
    // 分配存储空间
    pfifo->pCache =(unsigned char *)malloc(sizeof(unsigned char )* (iSize));
    // 初始化队列属性
    if(pfifo->pCache == NULL)
    {
        free(pfifo);
        return  (FIFO_HAND)ERROR_MEM;
    }
    pthread_mutex_init(&pfifo->pMutex,NULL);

    pfifo->iProtectionSize = iProctection;
    pfifo->iCacheSize = iSize;
    pfifo->iPosFront = pfifo->iPosBack = 0;

    return   (FIFO_HAND)pfifo;
}
// 队列中添加快数据
int fifo_PushRang(FIFO_HAND hFifo,unsigned char * aArray,int iSize)
{
    int i = 0;
    int iFreeSize = 0;
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;
    // 句柄检查
    if(pfifo == NULL)
    {
        fprintf(stderr,"fifo_PushRang:FIFO HAND IS NULL \n");
        return ERROR_NULL_HANDLE;
    }
    // 参数检查
    if(aArray == NULL || iSize <0)
    {
        fprintf(stderr,"fifo_PushRang:Push Range error %d ,%d\n",iSize,aArray);
        return ERROR_PARAM;
    }
    iFreeSize = fifo_get_cache_free(hFifo);
//	printf("iFreeSize = %d, iSize = %d \n",iFreeSize,iSize);


    if(iFreeSize > iSize)
    {
        fifo_copy_to(pfifo,aArray,iSize);
        return iSize;
    }
    else
    {
        fifo_copy_to(pfifo,aArray,iFreeSize);
        return iFreeSize;
    }



    for(i=0;i<iSize;i++)
    {// 队列满
        if(fifo_IsFull(hFifo))
        {
            fprintf(stderr,"fifo_PushRang:FIFO IS FULL %d-%d\n",i,iSize);
            pthread_mutex_lock(&pfifo->pMutex);
            pfifo->iPosFront = (pfifo->iPosBack+1)%pfifo->iCacheSize;
            pfifo->iPosBack = (pfifo->iPosBack+1)%pfifo->iCacheSize;
            pthread_mutex_unlock(&pfifo->pMutex);
        }
//		pthread_mutex_lock(&pfifo->pMutex);
        if(pfifo->iPosBack>(pfifo->iCacheSize-1))
            pfifo->iPosBack = 0;
        pfifo->pCache[pfifo->iPosBack++] = aArray[i];
        if(pfifo->iPosBack>(pfifo->iCacheSize-1))
            pfifo->iPosBack = 0;
//		pthread_mutex_unlock(&pfifo->pMutex);
    }
    return i;
}
// 队列添加一个单元数据
int fifo_Push(FIFO_HAND hFifo,unsigned char ch)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    if(fifo_IsFull(hFifo)) return ERROR_FIFO_FULL;
    pthread_mutex_lock(&pfifo->pMutex);
    if(pfifo->iPosBack>(pfifo->iCacheSize-1))
            pfifo->iPosBack = 0;

    if(fifo_IsFull(hFifo))
        pfifo->iPosFront = (pfifo->iPosBack+1)%pfifo->iCacheSize;

    pfifo->pCache[pfifo->iPosBack++] = ch;

    if(pfifo->iPosBack>(pfifo->iCacheSize-1))
            pfifo->iPosBack = 0;

    pthread_mutex_unlock(&pfifo->pMutex);

    return 0;
}
// 从队列开始位置查找字符串
int fifo_FindStrs(FIFO_HAND hFifo,unsigned char * strs,int num)
{
    int curFix;
    int index = 0;
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    if(strs == NULL || num <0) return ERROR_PARAM;

    curFix = pfifo->iPosFront;
    // 从起始位置 寻找到队列尾
    pthread_mutex_lock(&pfifo->pMutex);
    while(curFix -pfifo->iPosBack!=0)
    {	// 匹配字符
        if(pfifo->pCache[curFix] == strs[index])
            index ++;
        else
            index = 0;

        curFix = (curFix+ 1)%pfifo->iCacheSize;
        // 超过限制尺寸
        if(index == num)	break;
    }
    pthread_mutex_unlock(&pfifo->pMutex);
    if(index == num)
        return curFix;
    else
        return -1;
}
// 获得队列尾 索引标号
int fifo_GetEndFix(FIFO_HAND hFifo)
{
    struct fifo_stru * pfifo = (struct fifo_stru *) hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    return pfifo->iPosBack;
}
// 从起始位置 到结束标号 块数据读取 读过程中出队列
int fifo_GetBlock(FIFO_HAND hFifo,int iEndFix,char * buf,int *num)
{
    int count = 0;
    struct fifo_stru * pfifo = (struct fifo_stru *) hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    if(iEndFix <0 || buf == NULL || num == NULL || (*num)<0) return ERROR_PARAM;

    pthread_mutex_lock(&pfifo->pMutex);
    while((pfifo->iPosFront - iEndFix != 0 )&&( pfifo->iPosFront- pfifo->iPosBack != 0))
    {
        if(count >= (*num))	 return 0;
        buf[count] = pfifo->pCache[pfifo->iPosFront];
        pfifo->iPosFront = (pfifo->iPosFront+1)%pfifo->iCacheSize;
        count ++;
    }
    pthread_mutex_unlock(&pfifo->pMutex);
    buf[count] = 0;
    *num = count;
    return 0;
}
// 判断队列是否为空
int fifo_IsEmpty(FIFO_HAND hFifo)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;
    int iResult = 0;
    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    pthread_mutex_lock(&pfifo->pMutex);
    iResult = (pfifo->iPosFront-pfifo->iPosBack);
    pthread_mutex_unlock(&pfifo->pMutex);

    return iResult == 0;
}
// 判断队列是否满
int fifo_IsFull(FIFO_HAND hFifo)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;
    int value =0;
    int flag = 0 ;
    if(pfifo ==NULL)
        return ERROR_NULL_HANDLE;
    pthread_mutex_lock(&pfifo->pMutex);
    value = pfifo->iPosFront - pfifo->iPosBack;
    flag = (value == 1 || value == (1-pfifo->iCacheSize));
    pthread_mutex_unlock(&pfifo->pMutex);

    return flag;
}
// 出队列 出一个单元数据
int fifo_Pop(FIFO_HAND hFifo,unsigned char * pCh)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL)
        return ERROR_NULL_HANDLE;
    if(pCh == NULL) return ERROR_PARAM;

    if(fifo_IsEmpty(hFifo)) return ERROR_FIFO_EMPTY;

    pthread_mutex_lock(&pfifo->pMutex);
    *pCh =  pfifo->pCache[pfifo->iPosFront];
    pfifo->iPosFront = (pfifo->iPosFront+1)%pfifo->iCacheSize;
    pthread_mutex_unlock(&pfifo->pMutex);
    return 0;
}
int fifo_PopRang(FIFO_HAND hFifo, unsigned char * chArray,int * iNum)
{
    struct fifo_stru * pfifo = (struct fifo_stru * )hFifo;
    int i = 0;
    char ch;
    int iCacheSize = 0;
    if(pfifo == NULL)	return ERROR_NULL_HANDLE;


    if(fifo_IsEmpty(hFifo))
    {
        (*iNum) = 0;
        chArray[0] = 0;
        return 0;
    }

    iCacheSize = fifo_GetLength(pfifo);


    if(iCacheSize > (*iNum))
    {
        fifo_copy_from(pfifo,chArray,(*iNum));
        return (*iNum);
    }
    else
    {
        fifo_copy_from(pfifo,chArray,iCacheSize);
        return iCacheSize;
    }


    while(i<(*iNum))
    {
//		pthread_mutex_lock(&pfifo->pMutex);
        chArray[i] = pfifo->pCache[pfifo->iPosFront];
        pfifo->iPosFront = (pfifo->iPosFront+1)%pfifo->iCacheSize;
        i++;
//		pthread_mutex_unlock(&pfifo->pMutex);
        if(fifo_IsEmpty(hFifo)) break;
    }
    chArray[i] = 0;
    (*iNum) = i;
    return 0;
}
// 弹出 两个单元数据
int fifo_ReadWord(FIFO_HAND hFifo, short int * pValue)
{
    int i = 0;
    unsigned char ch[2];
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    if(pValue == NULL) return ERROR_PARAM;

    if(fifo_GetLength(hFifo)<2) return ERROR_NOT_ENOUGH;

    pthread_mutex_lock(&pfifo->pMutex);
    for(i = 0;i<2;i++)
    {
        if(((pfifo->iPosFront+i)%pfifo->iCacheSize) == pfifo->iPosBack)	break;
        ch[i] = pfifo->pCache[(pfifo->iPosFront+i)%pfifo->iCacheSize];
    }
    pthread_mutex_unlock(&pfifo->pMutex);
    memcpy(pValue,ch,2);

    return i;
}
// 读取队列中指定字节数的值，并不做弹出
int fifo_Read(FIFO_HAND hFifo, unsigned char * chArray,int iNum)
{
    int i = 0;
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    if(chArray == NULL|| iNum <0) return ERROR_PARAM;

    if(fifo_GetLength(hFifo)<iNum) return ERROR_NOT_ENOUGH;

    pthread_mutex_lock(&pfifo->pMutex);
    for(i = 0;i<iNum;i++)
    {
        if(((pfifo->iPosFront+i)%pfifo->iCacheSize) == pfifo->iPosBack)
            break;
        chArray[i] = pfifo->pCache[(pfifo->iPosFront+i)%pfifo->iCacheSize];
    }
    pthread_mutex_unlock(&pfifo->pMutex);
    chArray[i] = 0;
    return i;
}
// 找回制定字节的数据 注意危险操作 保证读取后 再回退
int fifo_Backspace(FIFO_HAND hFifo,int iNum)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    if(iNum <0) return ERROR_PARAM;

    if((pfifo->iPosFront - iNum)<0)
        pfifo->iPosFront = (pfifo->iPosFront-iNum)+pfifo->iCacheSize;
    else
        pfifo->iPosFront = pfifo->iPosFront-iNum;

    return 0;
}
// 从头开始删除 iDelNum 个值
int fifo_Remove(FIFO_HAND hFifo,int iDelNum)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    if(iDelNum<0) return ERROR_PARAM;

    if(fifo_GetLength(hFifo) <iDelNum)
        pfifo->iPosFront = pfifo->iPosBack;
    else
        pfifo->iPosFront = (pfifo->iPosFront+iDelNum)% pfifo-> iCacheSize;

    return 0;
}
// 删除队列中所有值
int fifo_RemoveAll(FIFO_HAND hFifo)
{
    struct fifo_stru  * pfifo = (struct fifo_stru *) hFifo;

    if(pfifo == NULL) return ERROR_NULL_HANDLE;

    pfifo->iPosFront = pfifo->iPosBack = 0;

    return 0;
}
int fifo_PopWord(FIFO_HAND hFifo,short *value)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;
    unsigned char ch[2];
    int iResult = 0;
    if(pfifo == NULL)
        return ERROR_NULL_HANDLE;

    if((iResult = fifo_Pop(hFifo,&ch[0])==0) && (iResult = fifo_Pop(hFifo,&ch[1]) == 0))
    {
        memcpy(value,ch,2);
    }
    else
        return iResult;
    return 0;
}
int fifo_GetIndex(FIFO_HAND hFifo)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL)
        return ERROR_NULL_HANDLE;

    return pfifo->iPosFront;
}
int fifo_Distance(FIFO_HAND hFifo,int iBegin,int iEnd)
{
    int value = 0;
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    int i = 0;

    i = iEnd - iBegin;
    value = (i<0)?( i+pfifo->iCacheSize ): i;

    return value;
}
int fifo_GetOfFromCount(FIFO_HAND hFifo,int iBegin)
{
    int value = 0;
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;
    int i = 0;

    if(pfifo == NULL)
        return ERROR_NULL_HANDLE;

    i = pfifo->iPosFront - iBegin;
    value = (i<0)?( i+pfifo->iCacheSize ): i;

    return value;
}
int fifo_IsProtection(FIFO_HAND hFifo)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL)
        return ERROR_NULL_HANDLE;

    if(fifo_GetLength(hFifo) <=pfifo->iProtectionSize)
        return 1;
    else
        return 0;
}
int fifo_GetLength(FIFO_HAND hFifo)
{
    int value = 0;

    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL)
        return ERROR_NULL_HANDLE;

    value = pfifo->iPosBack - pfifo->iPosFront;

    value = (value<0)?(value+pfifo->iCacheSize) : value;

    return value;
}
int fifo_Destory(FIFO_HAND hFifo)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo == NULL)
        return ERROR_NULL_HANDLE;

    pthread_mutex_lock(&pfifo->pMutex);
    if(pfifo->pCache != NULL) free(pfifo->pCache);
    pfifo->pCache = NULL;
    pfifo->iPosFront = pfifo->iPosBack = 0;
    pthread_mutex_unlock(&pfifo->pMutex);
    pthread_mutex_destroy(&pfifo->pMutex);
    free(pfifo);
    hFifo = NULL;

    return 0;
}
int fifo_Clean(FIFO_HAND hFifo)
{
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo== NULL)
        return ERROR_NULL_HANDLE;

    pfifo->iPosFront = pfifo->iPosBack = 0;

    return 0;

}
int fifo_get_cache_free(FIFO_HAND hFifo)
{
    int iResult ;
    int iLength;
    struct fifo_stru * pfifo = (struct fifo_stru *)hFifo;

    if(pfifo== NULL) return ERROR_NULL_HANDLE;

    iLength = fifo_GetLength(hFifo);

    pthread_mutex_lock(&pfifo->pMutex);
    iResult = pfifo->iCacheSize-iLength-1;
    pthread_mutex_unlock(&pfifo->pMutex);

    return iResult;
}
int fifo_copy_to(struct fifo_stru* stru,unsigned char * buf,int len)
{
    unsigned char * p = buf;

    while(len >0)
    {
        int size = len>(stru->iCacheSize-stru->iPosBack)?(stru->iCacheSize-stru->iPosBack):len;
        memcpy(&stru->pCache[stru->iPosBack],p,size);
        stru->iPosBack= (stru->iPosBack+size)%stru->iCacheSize;

        p+=size;
        len -=size;

    }
    return 0;
}
int fifo_copy_from(struct fifo_stru* stru,unsigned char * buf,int len)
{
    unsigned char * p = buf;

    while(len >0)
    {
        int size = len>(stru->iCacheSize-stru->iPosFront)?(stru->iCacheSize-stru->iPosFront):len;
        memcpy(p,&stru->pCache[stru->iPosFront],size);
        stru->iPosFront= (stru->iPosFront+size)%stru->iCacheSize;

        p+=size;
        len -=size;
    }
    return 0;
}
