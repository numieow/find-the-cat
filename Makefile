ftc : ftc.o MegaMimes.o
	gcc -o ftc ftc.o MegaMimes.o
	rm -f *.o

ftc.o : src/ftc.c src/ftc.h
	gcc -c src/ftc.c 

MegaMimes.o : src/MegaMimes.c src/MegaMimes.h
	gcc -c src/MegaMimes.c 

clean :
	rm -f *.o ftc MegaMimes
