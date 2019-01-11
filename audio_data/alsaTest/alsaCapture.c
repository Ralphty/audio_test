
#ifdef DEV_ALSA
#include <alsa/asoundlib.h>
#else
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#endif

#include <linux/soundcard.h>
#include <alsaCapture.h>

#ifdef DEV_ALSA
snd_pcm_t *capture_handle;
#else
int capture_handle;
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int As_Init(int iCaptureFormat,int iRate,int iChannel)
{
	int err;
	int iResult;
#ifdef DEV_ALSA
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t sndFormat = SND_PCM_FORMAT_UNKNOWN;
#else
	int sndFormat = AFMT_S16_LE;
#endif

#ifdef DEV_ALSA
	if ((err = snd_pcm_open (&capture_handle,"default", SND_PCM_STREAM_CAPTURE, 0)) < 0) 
	{
		fprintf (stderr,"cannot open audio device %s (%s)\n", "default",snd_strerror (err));
		return -1;
	}
#else
	if((capture_handle = open("/dev/dsp",O_RDWR)) == -1)
	{
		perror("open audio device");
		return -1;
	}
#endif

#ifdef DEV_ALSA
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) 
	{
		fprintf (stderr,"cannot allocate hardware parameter structure (%s)\n",snd_strerror (err));
		return -1;
	}
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) 
	{
		fprintf (stderr,"cannot initialize hardware parameter structure (%s)\n",snd_strerror (err));
		return -1;
	}
	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) 
	{
		fprintf (stderr,"cannot set access type (%s)\n",snd_strerror (err));
		return -1;
	}
	
	switch(iCaptureFormat)
	{
		case 16: sndFormat = SND_PCM_FORMAT_S16_LE; break;
		default:
			sndFormat = SND_PCM_FORMAT_UNKNOWN;
			break;
	}
	
	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, sndFormat)) < 0) 
	{
		fprintf (stderr,"cannot set sample format (%s)\n",snd_strerror (err));
		return -1;
	}
 	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &iRate, 0)) < 0) 
	{
		fprintf (stderr, "cannot set sample rate (%s)\n",snd_strerror (err));
		return -1;
	}
	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, iChannel)) < 0) 
	{
		fprintf (stderr,"cannot set channel count (%s)\n",snd_strerror (err));
		return -1;
	}
	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) 
	{
		fprintf (stderr,"cannot set parameters (%s)\n",snd_strerror (err));
		return -1;
	}
	snd_pcm_hw_params_free (hw_params);
	
	if ((err = snd_pcm_prepare (capture_handle)) < 0) 
	{
		fprintf (stderr,"cannot prepare audio interface for use (%s)\n",snd_strerror (err));
		return -1;
	}
#else
	switch(iCaptureFormat)
	{
		case 16: sndFormat = AFMT_S16_LE; break;
		default:
			sndFormat = AFMT_S16_LE;
			break;
	}
	iResult = ioctl(capture_handle,SNDCTL_DSP_SETFMT,&sndFormat);
	if(iResult == -1)
	{
		perror("set format");
		return -1;
	}
	iResult = ioctl(capture_handle,SNDCTL_DSP_SPEED,&iRate);
	if(iResult == -1)
	{
		perror("set rate speed");
		return -1;
	}
	iResult = ioctl(capture_handle,SNDCTL_DSP_CHANNELS,&iChannel);
	if(iResult == -1)
	{
		perror("set channel error");
		return -1;
	}
#endif
//	snd_pcm_nonblock(capture_handle, 1);
	return 0;
}
int As_Read(short *pBuf,int iReadNum)
{
	int err;
#ifdef DEV_ALSA
	err = snd_pcm_readi (capture_handle,pBuf,iReadNum);
	//if ((err = snd_pcm_readi (capture_handle,pBuf,iReadNum)) != iReadNum) 
	//{
	//	fprintf (stderr,"read from audio interface failed-%d (%s)\n",err,snd_strerror (err));
	//	return -1;
	//}
	if(err == -EPIPE)
	{
		printf("underrun occurred \n");
	}
	err*=2;
#else
	if((err = read(capture_handle,pBuf,iReadNum*2)) == -1)
	{
		perror("read data error");
		return -1;
	}
	return err/2;
#endif
	return err;	
}
int As_Close()
{
#ifdef DEV_ALSA
	snd_pcm_close (capture_handle);
#else
	close(capture_handle);
#endif
	return 0;
}
