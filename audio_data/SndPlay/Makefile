COMP=/home/zhaokai/workspace/cross-env/Cross-GCC/fsl-linaro-toolchain/bin/arm-linux-gcc
#COMP=gcc
LIB_PATH=-L/home/zhaokai/MyBuildLib/alsa-lib-1.0.26/.build/lib
INC_PATH=-I/home/zhaokai/MyBuildLib/alsa-lib-1.0.26/.build/include
#INC_PATH=
#LIB_PATH=-L/usr/lib64
all:SndPlay
SndPlay: main.o soundJNI.o fifo.o list_lib.o
	$(COMP) main.o soundJNI.o fifo.o list_lib.o -o SndPlay $(INC_PATH) $(LIB_PATH) -lasound -lpthread
main.o:	main.c
	$(COMP) -fPIC -D_REENTRANT -I./ $(INC_PATH) -c main.c
soundJNI.o:soundJNI.c fifo.c list_lib.c
	$(COMP) -fPIC -D_REENTRANT -I./ $(INC_PATH) -c soundJNI.c
fifo.o:fifo.c
	$(COMP) -fPIC -D_REENTRANT -c fifo.c
list_lib.o:list_lib.c
	$(COMP) -fPIC -D_REENTRANT -c list_lib.c
clean:
	rm -f *.o SoundRecord
