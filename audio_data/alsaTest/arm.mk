

AUDIO = DEV_OSS
#AUDIO = DEV_ALSA

INCPATH = -I./ 

ifeq "$(AUDIO)" "DEV_ALSA"

#INCPATH =  -I./ /home/zhaokai/MyWorkspace/environment/alsa/include
#LD_LIBRARY_PATH= -L/home/zhaokai/MyWorkspace/environment/alsa/lib -lasound
LD_LIBRARY_PATH=-lasound
endif
GCC=gcc
#GCC = /home/zhaokai/MyWorkspace/environment/GCC/CortexA8/4.5.1/bin/arm-linux-gcc

include Makefile
