chash: chash.o hashtable.o
		gcc -o chash chash.o hashtable.o rwlock.o -pthread

chash.o: chash.c hashtable.h
		gcc -c chash.c -pthread

hashtable.o: hashtable.c hashtable.h
		gcc -c hashtable.c -pthread

clean:
		rm -f *.o chash hash.log output.txt