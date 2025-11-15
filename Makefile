chash: chash.o hashtable.o rwlock.o
		gcc -o chash chash.o hashtable.o rwlock.o -pthread

chash.o: chash.c chash.h hashtable.h rwlock.h
		gcc -c chash.c -pthread

hashtable.o: hashtable.c chash.h hashtable.h rwlock.h
		gcc -c hashtable.c -pthread

rwlock.o: rwlock.c rwlock.h
		gcc -c rwlock.c -pthread

clean:
		rm -f *.o chash hash.log output.txt