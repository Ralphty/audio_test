.PHONY:all

all: alsaCapture

alsaCapture:main.o alsaCapture.o
	$(GCC) -o alsaCapture main.o alsaCapture.o $(LD_LIBRARY_PATH)
main.o:main.c
	$(GCC) -c main.c $(INCPATH)
alsaCapture.o:alsaCapture.c
	$(GCC) -c alsaCapture.c $(INCPATH) -D$(AUDIO)
clean:
	rm alsaCapture *.o -f
