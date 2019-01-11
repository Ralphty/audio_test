/*
 * soundJNI.c
 * This file is part of <program name>
 *
 * Copyright (C) 2015 - zhaokai
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>. If not, see <http://www.gnu.org/licenses/>.
 */
#include <soundJNI_c.h>
#include <fifo.h>
#include <list_lib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <sys/time.h>


#define FRAME_SIZE 32

char g_strError[200] = {0};

struct stru_snd
{
	snd_pcm_t * snd_pcm_hand;					// 声卡虚拟设备句柄
	snd_pcm_hw_params_t *snd_hw_params;  			// 声卡配置参数变量
	int snd_channel;							// 通道数
	int snd_rate;							// 采样率
	int snd_bit_width;						// 为宽 大端小端 有无符号 的值
	int snd_bit;							// 为宽位数
	int snd_frames;							// 写入帧大小
	FIFO_HAND snd_data_cache;					// 数据缓存
};

struct stru_snd_hand
{
	pthread_t pThread_hand;						// 写入线程句柄
	struct stru_snd snd_dev;					// 声卡属性结 构
	bool snd_run_flag;
	int snd_id;								// 句柄ID 可多句柄同时工作
	pthread_cond_t pCond;
	pthread_mutex_t pMutex;
};

static int g_max_link_num = 0;					// 最大音频链路数目
static int g_cache_msec = 0;						// 缓存音频时间（MS）
static int g_regedit_in= 0;

static struct stru_snd_hand * g_pManageList = NULL;		// 链路管理句柄
static int 	g_ManageLen = 0;						// 已注册的链路数
/*================================================public interface====================================================*/
/***********************************************************
			初始化 语音管理器
iMaxLink	链路最大数
iCacheTimes 数据缓存数目
返回： 成功，失败
***********************************************************/
static bool  snd_init_manager(int iMaxLink,int iCacheTimesMS);

/***********************************************************
			新建 一套语音播放链路
iChannel 	通道数
iReate 	采样率
bSigned	采样数据是否有符号 1有符号 0 无符号
iBits		采样位宽 8/16/24/32   **GSM 等其他选项暂时不支持
bBig		采样数据是否为大端小端 	1为大端 0 为小端
iFrame	播放数据最小单元 为每次播放块大小
返回：	语音链路ID，<=0失败
***********************************************************/
static int snd_new_link(int iChannel,int iRate,bool bSigned,int iBits,bool bBig,int iFrame);

/***********************************************************
			向连率中写入数据
snd_id	链路ID
buf		写入缓存
len		写入数据
返回：成功写入数据尺寸，失败	<=0		
***********************************************************/
static int snd_write_link(int snd_id,unsigned char * buf,int len);

/***********************************************************
			向连率中写入数据 等待数据全部写入缓存
snd_id	链路ID
buf		写入缓存
len		写入数据
返回：成功写入数据尺寸，失败	<=0			
***********************************************************/
static int snd_write_link_wait_cache(int snd_id,unsigned char * buf,int len);

/***********************************************************
			释放一条语音链路
snd_id	链路ID
返回：		>=0成功，失败<0
***********************************************************/
static bool snd_destory_link(int snd_id);

/***********************************************************
			释放 语音管理器
返回：		成功，失败		
***********************************************************/
static bool snd_destory_manager();

/***********************************************************
			获得数据缓存的剩余尺寸
snd_id	链路ID
返回：		返回剩余尺寸		
***********************************************************/
static int snd_get_cache_free(int snd_id);

/***********************************************************
			获得写入到缓存中的数据尺寸
snd_id	链路ID
返回：		返回数据尺寸	
***********************************************************/
static int snd_get_cahe_size(int snd_id);


/*================================================private interface===================================================*/
/***********************************************************
			初始化音频设备
返回：		成功，失败		
***********************************************************/
static bool snd_init_card(struct stru_snd * pSnd,int iChannel,int iRate,bool bSigned,int iBits,bool bBig,int iFrames);

/***********************************************************
			初始化音频设备
返回：		成功，失败		
***********************************************************/
static snd_pcm_format_t snd_get_bit_width(bool bSigned,int iBits,bool bBig);

static struct stru_snd_hand * snd_get_node_for_id(struct stru_snd_hand * pList,int id);

static int snd_get_link_new_id(struct stru_snd_hand * pList);

static void * snd_thread_write(void * arg);

static void snd_del_node_by_id(int snd_id,LIST_HANDLER_PTR* pList);

static int snd_get_cache_size(int snd_id);

static struct stru_snd_hand* snd_init_manager_hand(int iMaxLink);

static int snd_destory_hand(struct stru_snd_hand * pNode);

static int snd_thread_msleep(int time,pthread_mutex_t mutex,pthread_cond_t cond);

/*================================================C INTERFACE=========================================================*/
int write_link(int snd_id,unsigned char * buf,int len)
{
	return snd_write_link(snd_id, buf, len);
}
int  init_manager(int iMaxLink,int iCacheTimes)
{
	return  snd_init_manager(iMaxLink,iCacheTimes);
}
int new_link(int iChannel,int iRate,int bSigned,int iBits,int bBig,int iFrames)
{
	return snd_new_link(iChannel,iRate,bSigned,iBits,bBig,iFrames);
}
int destory_link(int snd_id)
{
	return snd_destory_link(snd_id);
}
int destory_manager()
{
	return snd_destory_manager();
}
int 	get_cache_size(int snd_id)
{
	snd_get_cahe_size(snd_id);
}
int 	write_link_by_cache(int snd_id,unsigned char * buf,int len)
{
	return snd_write_link_wait_cache(snd_id, buf, len);
}
/*====================================================================================================================*/

bool  snd_init_manager(int iMaxLink,int iCacheTimes)
{
	int i = 0;
	
	if(g_pManageList == NULL)
	{												// 管理连表 是否为空 空建立  非空释放并建立
		g_pManageList = snd_init_manager_hand(iMaxLink);
		if(g_pManageList == NULL)
			return false;
	}
	else
	{
		if(snd_destory_manager() == false ) goto loop;
		g_pManageList = snd_init_manager_hand(iMaxLink);
		if(g_pManageList == NULL)
			return false;
	}
	
	if(iMaxLink <=0 || iCacheTimes<=0)
	{
		fprintf(stderr,"param error \n");
		goto loop;	
	}	
	
	g_max_link_num = iMaxLink;
	g_cache_msec = iCacheTimes;
	fprintf(stdout,"init sound manager max_link= %d , save time data  = %d ms \n",iMaxLink,iCacheTimes);
	return true;
loop:
	if(g_pManageList != NULL)									// 释放管理连表
		snd_destory_manager();
	iMaxLink = 0;
	iCacheTimes = 0;
	
	return false;
	
}
int snd_new_link(int iChannel,int iRate,bool bSigned,int iBits,bool bBig,int iFrame)
{
//	struct stru_snd_hand devNode;
	struct stru_snd_hand *pNode;
	snd_pcm_hw_params_t *hw_params;
	pthread_condattr_t attr;
	int dev_id;
	int iResult = 0;
	
	
	if(iChannel <=0 || iRate <=0|| iBits <=0||iFrame<=0)
	{
		fprintf(stderr,"param error \n");
		return -1;
	}
	if(g_regedit_in > g_max_link_num)
	{
		fprintf(stderr,"snd_new_link:THE REGISTER LINK FULL\n");
		return -1;
	}
	
	dev_id =  snd_get_link_new_id(g_pManageList);
	if(dev_id < 0)
	{
		fprintf(stderr,"Get new id error \n",dev_id);
		return -1;
	}
	// 初始化声卡
	iResult = snd_init_card(&(g_pManageList[dev_id].snd_dev),iChannel,iRate,bSigned,iBits,bBig,iFrame);
	if(iResult <0)
	{
		fprintf(stderr,"Init Cart error \n");
		goto loops;
	}
	g_pManageList[dev_id].snd_run_flag = true;
	
//	if(pthread_create(&(g_pManageList[dev_id].pThread_hand),NULL,snd_thread_write,&g_pManageList[dev_id])!=0)
//	{
//		fprintf(stderr,"Create Thread error \n");
//		goto loops;
//	}
	g_pManageList[dev_id].snd_id = dev_id;
	g_regedit_in++;
	
	fprintf(stdout,"register a link in  reg id = %d channel = %d rate = %d signed = %d bits = %d big=%d frams = %d \n",
		dev_id,iChannel,iRate,bSigned,iBits,bBig,iFrame);
	
//	pthread_condattr_init(&attr);
	
//	pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
	
//	pthread_cond_init(&(g_pManageList[dev_id].pCond),&attr);
//	pthread_mutex_init(&(g_pManageList[dev_id].pMutex),NULL);
	
	return dev_id;
loops:
	printf("snd_new_link error \n");
	
	snd_destory_link(dev_id);
	return -1;
}
int snd_write_link(int snd_id,unsigned char * buf,int len)
{
	struct stru_snd_hand* pNode = NULL;
	
	
	return 0;
	if(g_pManageList == NULL)
	{
		return -1;
	}
	pNode = snd_get_node_for_id(g_pManageList,snd_id);
	if(pNode == NULL)
	{
		fprintf(stderr,"not find node \n");
		return -1;
	}
	if(buf == NULL|| len <=0)
	{
		fprintf(stderr,"param error \n");
		return 0;
	}
	
	return  fifo_PushRang(pNode->snd_dev.snd_data_cache,buf,len);
	
}
static int write_num = 0;
int snd_write_link_wait_cache(int snd_id,unsigned char * buf,int len)
{
	struct stru_snd_hand* pNode = NULL;
	unsigned char * p = buf;
	int iRemain = len;
	
	int sleepLong; 
	
	int iResult;
	if(g_pManageList == NULL)
		return -1;
	pNode = snd_get_node_for_id(g_pManageList,snd_id);
	if(pNode == NULL)
	{
		fprintf(stderr,"snd_wirte_link:not find node \n");
		return -1;
	}
	if(buf == NULL|| len <=0)
	{
		fprintf(stderr,"snd_write_link:param error \n");
		return 0;
	}
	
	iResult = snd_pcm_writei(pNode->snd_dev.snd_pcm_hand,buf,len/pNode->snd_dev.snd_channel/(pNode->snd_dev.snd_bit/8)); //pNode->snd_dev.snd_frames);

	if (iResult == -EPIPE)
	{
      		fprintf(stderr, "underrun occurred %s\n",snd_strerror(iResult));
                snd_pcm_prepare(pNode->snd_dev.snd_pcm_hand);
	}

	return iResult;
	
	
	
	sleepLong = (int)((pNode->snd_dev.snd_frames*pNode->snd_dev.snd_bit))/(double)pNode->snd_dev.snd_rate*1000;

	while(iRemain > 0) // && (flag == false)
	{
		struct timeval  tp1;
		struct timeval  tp2;
		long lost = 0;	
		struct timezone tzp;
		 
		int freeSize = snd_get_cache_free(snd_id);
		
//		gettimeofday(&tp1,&tzp);
		
//		printf("write fun %d \n",write_num++);
		if(freeSize ==0)
			usleep(sleepLong/2*1000);
		//	snd_thread_msleep(sleepLong/2,pNode->pMutex,pNode->pCond);
		else
		{

			if(iRemain<= freeSize)
			{
				fifo_PushRang(pNode->snd_dev.snd_data_cache,p,iRemain);
			//	gettimeofday(&tp2,&tzp);
				
			//	printf("lost time1 %d: %ld\n",iRemain,(long)(tp2.tv_sec-tp1.tv_sec)*1000000+(tp2.tv_usec-tp1.tv_usec));
				iRemain = 0;
				break;
			}
			else
			{	
				fifo_PushRang(pNode->snd_dev.snd_data_cache,p,(freeSize));
				iRemain -= (freeSize);
				p+= freeSize;
			//	gettimeofday(&tp2,&tzp);
			//	printf("lost time2 %d: %ld\n",freeSize,(long)(tp2.tv_sec-tp1.tv_sec)*1000000+(tp2.tv_usec-tp1.tv_usec));
				usleep(sleepLong/2*1000);
			}	
			//snd_thread_msleep(sleepLong/2,pNode->pMutex,pNode->pCond);
		}
	}
	return len - iRemain;
}
bool snd_destory_link(int snd_id)
{
	struct stru_snd_hand * pNode ;
	
	pNode = snd_get_node_for_id(g_pManageList,snd_id);
	if(pNode == NULL)
	{
		fprintf(stderr,"not find node \n");
		return -1;
	}

//	pthread_cond_destroy(&(pNode->pCond));
//	pthread_mutex_destroy(&(pNode->pMutex));

	if(snd_destory_hand(pNode) == true)
	{
		g_regedit_in--;
		return true;
	}
	else
		return false;
	return false;
}
bool snd_destory_manager()
{
	int iLength;
	int i;
	struct stru_snd_hand* pNode;
	
	if(g_pManageList == NULL) return false;
	
	for(i = 0;i<g_max_link_num;i++)
	{
		pNode = & g_pManageList[i];
		if(pNode->snd_id != -1)
		{
			if(snd_destory_link(pNode->snd_id) == false)
			{
				fprintf(stderr,"destory link error %d\n",i);
			}
			memset(pNode,0,sizeof(struct stru_snd_hand));
			pNode->snd_id = -1;
		}
	}
	free(g_pManageList);
	g_pManageList = NULL;
	g_regedit_in = 0;
	
	return true;
}
int snd_get_cahe_size(int snd_id)
{
	struct stru_snd_hand* pNode;
	
	pNode= snd_get_node_for_id(g_pManageList,snd_id);
	
	if(pNode == NULL) return -1;
	
	return fifo_GetLength(pNode->snd_dev.snd_data_cache);
}
int snd_get_cache_free(int snd_id)
{
	struct stru_snd_hand* pNode;
	
	pNode= snd_get_node_for_id(g_pManageList,snd_id);
	
	if(pNode == NULL) return -1;
	
	return fifo_get_cache_free(pNode->snd_dev.snd_data_cache);
}
/*====================================================================================================================*/
bool snd_init_card(struct stru_snd * pSnd,int iChannel,int iRate,bool bSigned,int iBits,bool bBig,int iFrames)
{
	int err;
	snd_pcm_uframes_t write_frames = iFrames;
	int dir = 0;
	snd_pcm_uframes_t tmp;
	pSnd->snd_pcm_hand = NULL;
	pSnd->snd_hw_params = NULL;
	pSnd->snd_data_cache = NULL;
	pSnd->snd_channel = iChannel;	// 通道数
	pSnd->snd_rate = iRate;		// 采样率
	pSnd->snd_bit_width = snd_get_bit_width(bSigned,iBits,bBig); // 位宽
	pSnd->snd_bit = iBits;
	pSnd->snd_frames = iFrames;
	
	if(pSnd->snd_bit_width  == SND_PCM_FORMAT_UNKNOWN)
	{
		fprintf(stderr,"get bit width error\n");
		goto loops;
	}
	pSnd->snd_data_cache = fifo_InitFifo((g_cache_msec*iRate*(iBits/8)*iChannel)/1000,0);// 缓存队列 ///1000
	
	if(pSnd->snd_data_cache == NULL)
	{
		fprintf(stderr,"alloc fifo error \n");
		goto loops;
	}
	/* Open PCM device for playback. */
	if ((err = snd_pcm_open (&pSnd->snd_pcm_hand,"default",SND_PCM_STREAM_PLAYBACK,0)) < 0) //plug:dmix
	{  	// 打开虚拟设备
      	fprintf(stderr,"cannot open audio device %s (%s)\n", "default",snd_strerror (err));  //plug:dmix
      	goto loops;  
   	} 
   	/* Allocate a hardware parameters object. */
   	if ((err = snd_pcm_hw_params_malloc (&pSnd->snd_hw_params)) < 0) 
   	{  	// 分配参数空间
      	fprintf(stderr,"cannot allocate hardware parameter structure (%s)\n",snd_strerror (err));  
      	goto loops;
   	} 
   	/* Fill it in with default values. */
   	if ((err = snd_pcm_hw_params_any (pSnd->snd_pcm_hand,pSnd->snd_hw_params)) < 0) 
   	{  	// 添入默认参数
      	fprintf(stderr,"cannot initialize hardware parameter structure (%s)\n",snd_strerror (err));  
      	goto loops; 
   	}
   	/* Interleaved mode */
   	if ((err = snd_pcm_hw_params_set_access(pSnd->snd_pcm_hand,pSnd->snd_hw_params,
   		SND_PCM_ACCESS_RW_INTERLEAVED))< 0) 
   	{  	// 设置PCM 访问方式
      	fprintf(stderr,"cannot set access type (%s)\n",snd_strerror (err));  
      	goto loops;  
   	}    
   	/* Signed 16-bit little-endian format */ 
	if ((err = snd_pcm_hw_params_set_format (pSnd->snd_pcm_hand,pSnd->snd_hw_params,
		 SND_PCM_FORMAT_S16_LE)) < 0) 
  	{  	// 设置播放位宽
      	fprintf(stderr,"cannot set sample format (%s)\n",snd_strerror (err));  
      	goto loops;
   	} 
   	/* Two channels (stereo) */
   	if ((err = snd_pcm_hw_params_set_channels(pSnd->snd_pcm_hand,pSnd->snd_hw_params,
   		pSnd->snd_channel)) < 0) 
   	{  	// 设置通道数
      	fprintf(stderr,"cannot set channel count (%s)\n",snd_strerror (err));  
      	goto loops;  
   	}  
  	/* 44100 bits/second sampling rate (CD quality) */
   	if ((err = snd_pcm_hw_params_set_rate_near(pSnd->snd_pcm_hand,pSnd->snd_hw_params,
   		&pSnd->snd_rate, 0)) < 0) 
   	{  	// 设置采样率
      	fprintf(stderr,"cannot set sample rate (%s)\n",snd_strerror (err));  
      	goto loops;  
   	}
	tmp = write_frames/2;
   	snd_pcm_hw_params_set_buffer_size_near(pSnd->snd_pcm_hand,pSnd->snd_hw_params,&write_frames);
	snd_pcm_hw_params_set_period_size_near(pSnd->snd_pcm_hand,pSnd->snd_hw_params,&tmp,0);   	
   	if ((err = snd_pcm_hw_params (pSnd->snd_pcm_hand,pSnd->snd_hw_params)) < 0) 
   	{  
      	fprintf(stderr,"cannot set parameters (%s)\n",snd_strerror (err));  
      	goto loops; 
   	}  
	//snd_pcm_nonblock(pSnd->snd_pcm_hand, 1);
   
        snd_pcm_prepare(pSnd->snd_pcm_hand);
	
   	return true;
loops:
	if(pSnd->snd_pcm_hand != NULL)
	{
		snd_pcm_drop(pSnd->snd_pcm_hand);
  		snd_pcm_close(pSnd->snd_pcm_hand);
  		pSnd->snd_pcm_hand  = NULL;
	}
	if(pSnd->snd_hw_params != NULL)
	{
		snd_pcm_hw_params_free (pSnd->snd_hw_params);
		pSnd->snd_hw_params = NULL;
	}
		
	if(pSnd->snd_data_cache != NULL)
	{
		fifo_Destory(pSnd->snd_data_cache);
		pSnd->snd_data_cache = NULL;
	}
	pSnd->snd_pcm_hand = 0;
	return false;
   	
}
snd_pcm_format_t snd_get_bit_width(bool bSigned,int iBits,bool bBig)
{
	switch(iBits)
	{
		case 8:
		{
			if(bSigned == true)
				return SND_PCM_FORMAT_S8;
			else
				return SND_PCM_FORMAT_U8;
		}break;
		case 16:
		{
			if(bSigned == true)
			{
				if(bBig == true)
					return SND_PCM_FORMAT_S16_BE;
				else
					return SND_PCM_FORMAT_S16_LE;
			}
			else
			{
				if(bBig == true)
					return SND_PCM_FORMAT_U16_BE;
				else
					return SND_PCM_FORMAT_U16_LE;
			}
		}break;
		case 24:
		{
			if(bSigned == true)
			{
				if(bBig == true)
					return SND_PCM_FORMAT_S24_BE;
				else
					return SND_PCM_FORMAT_S24_LE;
			}
			else
			{
				if(bBig == true)
					return SND_PCM_FORMAT_U24_BE;
				else
					return SND_PCM_FORMAT_U24_LE;
			}
		}break;
		case 32:
		{
			if(bSigned == true)
			{
				if(bBig == true)
					return SND_PCM_FORMAT_S32_BE;
				else
					return SND_PCM_FORMAT_S32_LE;
			}
			else
			{
				if(bBig == true)
					return SND_PCM_FORMAT_U32_BE;
				else
					return SND_PCM_FORMAT_U32_LE;
			}
		}break;
		
		default: return SND_PCM_FORMAT_UNKNOWN;
	}
}
int snd_get_link_new_id(struct stru_snd_hand * pList)
{
	int iLength;
	int i,j;
	struct stru_snd_hand * pNode;
	
	if(pList == NULL) return -1;
	
	if(g_regedit_in >= g_max_link_num) 
	{
		fprintf(stderr,"link manager is full\n");
		return -1;
	}
	
	for(i = 0;i<g_max_link_num;i++)
	{
		if(pList[i].snd_id == -1)
			return i;
	}
	return -1;
}
struct stru_snd_hand * snd_get_node_for_id(struct stru_snd_hand * pList,int id)
{
	int iLength;
	int i;
	struct stru_snd_hand* pNode;
	
	if(pList == NULL) return NULL;

	if(id>= g_max_link_num)	return NULL;
	if(pList[id].snd_id == -1) return NULL;
	return &pList[id];
}
static int num = 0;
void * snd_thread_write(void * arg)
{
	int err;
	struct stru_snd_hand* pNode = (struct stru_snd_hand *) arg;
	
	int frame_size = pNode->snd_dev.snd_frames*pNode->snd_dev.snd_channel*(pNode->snd_dev.snd_bit/8);
	unsigned char * pbuf;
	
	int sleepLong = (int)((pNode->snd_dev.snd_frames*pNode->snd_dev.snd_bit))/(double)pNode->snd_dev.snd_rate*1000;
	
	pbuf = (unsigned char *)malloc(frame_size);
	
	while(pNode->snd_run_flag)
	{
		int iLength = fifo_GetLength(pNode->snd_dev.snd_data_cache);
		break;
//		printf("thread %d\n",num++);
		if(iLength > frame_size)
		{
			int iResult;
			int iSize  = frame_size;
			
			fifo_PopRang(pNode->snd_dev.snd_data_cache,pbuf,&iSize);
			iResult = snd_pcm_writei(pNode->snd_dev.snd_pcm_hand,pbuf, pNode->snd_dev.snd_frames);
			if(iResult == -EPIPE)
			{
				if ((err = snd_pcm_prepare (pNode->snd_dev.snd_pcm_hand)) < 0) 
         			{  
  	   				fprintf(stderr,"cannot prepare audio interface for use (%s)\n",snd_strerror (err));  
    	  				pNode->snd_run_flag = false; 
    	  				break;
  	 			}  
			}
			else
				if(iResult <0)
					fprintf(stderr,"error from writei: %s\n",snd_strerror(iResult));
				else
					if(iResult != pNode->snd_dev.snd_frames)
						fprintf(stderr,"short write, write %d frames\n", iResult);
		}
		else
			usleep(sleepLong/4*1000);
		//snd_thread_msleep(sleepLong/2,pNode->pMutex,pNode->pCond);
	}
	free(pbuf);
}

void snd_del_node_by_id(int snd_id,LIST_HANDLER_PTR * pList)
{
	int iLength;
	int i;
	struct stru_snd_hand* pNode;
	
	if(pList == NULL) return ;
	if((iLength=LS_GetSize(pList)) == 0) return ;
	
	LS_Move_begin(pList);
	LS_Move_next(pList);
	pNode = LS_GetData(pList);
	
	while(pNode != NULL)
	{
		if(pNode->snd_id == snd_id)
		{
			LS_Del(pList);
			return;
		}
	
		LS_Move_next(pList);
		pNode = LS_GetData(pList);
	}
	
	return;
}
struct stru_snd_hand * snd_init_manager_hand(int iMaxLink)
{
	struct stru_snd_hand * pList;
	int i;
	pList = malloc(sizeof(struct stru_snd_hand )* iMaxLink);

	if(pList == NULL)
	{
		fprintf(stderr,"In snd_init_manager_hand mallock mem error \n");
		return NULL;
	}
	for(i =0 ;i<iMaxLink;i++)
	{
		memset(&pList[i],0,sizeof(struct stru_snd_hand));
		pList[i].snd_id = -1;
	}
	return pList;
}
int snd_destory_hand(struct stru_snd_hand * pNode)
{
	if(pNode == NULL) return false;
	
//	if(pNode->snd_run_flag == true)
//	{
		pNode->snd_run_flag= false;							// 停止写声卡线程
//		pthread_join(pNode->pThread_hand,NULL);
//	}
	
	if(pNode->snd_dev.snd_data_cache != NULL)
		fifo_Destory(pNode->snd_dev.snd_data_cache);				// 释放缓存
	
	if(pNode->snd_dev.snd_pcm_hand != 0)
	{
		snd_pcm_drop(pNode->snd_dev.snd_pcm_hand);				// 结束声卡写入
  		snd_pcm_close(pNode->snd_dev.snd_pcm_hand);				// 关闭声卡文件
  	
  		snd_pcm_hw_params_free(pNode->snd_dev.snd_hw_params);			// 释放声卡参数
	}
	memset(pNode,0,sizeof(struct stru_snd_hand));  
  	pNode->snd_id = -1;									// 释放ID号	

  	return true;
	
}
int snd_thread_msleep(int times,pthread_mutex_t mutex,pthread_cond_t cond)
{
	struct timespec tmsp;

//	time = 100;
//	printf("sleep time s : %d\n",times);
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);

//	times = 1000;
//	tmsp.tv_nsec = (time*1000000)%1000000000;
//	tmsp.tv_sec = (time*1000000)/1000000000;
//	tv.tv_sec+=5;
//	printf("nsec:%ld\n",tv.tv_nsec);
//	printf("sec: %ld\n",tv.tv_sec);
	tv.tv_nsec += (times*1000000)%1000000000;
	tv.tv_sec+=(times*1000000)/1000000000;
//	printf("nsec:%ld\n",tv.tv_nsec);
//	printf("sec: %ld\n",tv.tv_sec);
//	tv.tv_sec+=0;// (time*1000000)/1000000000;
	
//	pthread_mutex_lock(&mutex);
	
//	pthread_cond_timedwait(&cond,&mutex,&tv);
//	perror("sleep:");	
//	pthread_mutex_unlock(&mutex);
}

