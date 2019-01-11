

AUDIO = DEV_ALSA

INCPATH = -I./ 

ifeq "$(AUDIO)" "DEV_ALSA"
LD_LIBRARY_PATH=-lasound
endif
GCC=gcc
include Makefile
