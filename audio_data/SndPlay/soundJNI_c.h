
#ifndef SOUNDJNI_C_H
#define SOUNDJNI_C_H

//typedef enum{false = 0,true =1} bool;

typedef char bool;
#define true 1
#define false 0

int  init_manager(int iMaxLink,int iCacheTimes);
int 	write_link(int snd_id,unsigned char * buf,int len);
int 	new_link(int iChannel,int iRate,int bSigned,int iBits,int bBig,int iFrames);
int 	write_link_by_cache(int snd_id,unsigned char * buf,int len);
int 	destory_link(int snd_id);
int 	destory_manager();
int 	get_cache_size(int snd_id);

#endif
