all: chash

# just in case we don't have hash.log OR output.txt
create_files:
	touch hash.log
	touch output.txt

chash: chash.o hashtable.o rwlock.o
		gcc -o chash chash.o hashtable.o rwlock.o -pthread

chash.o: chash.c chash.h hashtable.h rwlock.h
		gcc -c chash.c -pthread

hashtable.o: hashtable.c hashtable.h
		gcc -c hashtable.c -pthread

rwlock.o: rwlock.c rwlock.h
		gcc -c rwlock.c -pthread

clean:
		rm -f *.o chash hash.log output.txt