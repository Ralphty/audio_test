#ifndef ALSACAPTURE_H
#define ALSACAPTURE_H
int As_Init(int iCaptureFormat,int iRate,int iChannel);
int As_Read(short *pBuf,int iReadNum);
int As_Close();
#endif
