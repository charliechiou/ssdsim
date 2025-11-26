# ssdsim linux support
all:ssd 
	
clean:
	rm -f ssd *.o *~
.PHONY: clean

ssd: ssd.o avlTree.o flash.o initialize.o pagemap.o ecc.o     
	cc -g -o ssd ssd.o avlTree.o flash.o initialize.o pagemap.o ecc.o -lm -Wno-error=implicit-function-declaration -fcommon
ssd.o: flash.h initialize.h pagemap.h
	gcc -c -g ssd.c -Wno-error=implicit-function-declaration -fcommon
flash.o: pagemap.h ecc.h
	gcc -c -g flash.c -Wno-error=implicit-function-declaration -fcommon	
initialize.o: avlTree.h pagemap.h
	gcc -c -g initialize.c -Wno-error=implicit-function-declaration -fcommon
pagemap.o: initialize.h
	gcc -c -g pagemap.c -Wno-error=implicit-function-declaration -fcommon
avlTree.o: 
	gcc -c -g avlTree.c -Wno-error=implicit-function-declaration -fcommon
ecc.o: ecc.h initialize.h
	gcc -c -g ecc.c -Wno-error=implicit-function-declaration -fcommon

