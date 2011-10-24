#DEFS = -DDEBUG
#OPT  = -g

all: clean binsearch test

clean:
	rm -f acbgrep test
	
binsearch:
	gcc ${DEFS} ${OPT} ./src/ahocorasick/aho-corasick.c ./src/ahocorasick/support/list.c  ./src/libhexstring.c ./src/libach.c ./src/acbgrep.c -o acbgrep

test:
	gcc -DTEST ${DEFS} ${OPT} ./src/ahocorasick/aho-corasick.c ./src/ahocorasick/support/list.c  ./src/libhexstring.c ./src/libach.c ./src/acbgrep.c src/test.c -o test


tests:
	dd if=/dev/urandom of=testfile bs=1024k count=5
	./test testfile
	./acbgrep testfile 3031323334 35363738

