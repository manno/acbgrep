SRCFILES = src/libac.c src/libhexstring.c
#DEFS = -DDEBUG
#OPT  = -g

all: binsearch test

binsearch:
	cd src/ahocorasick && gcc ${DEFS} aho-corasick.c -c
	rm -f acbgrep
	gcc ${OPT} ${DEFS} -DBINSEARCH src/acbgrep.c ${SRCFILES} src/ahocorasick/aho-corasick.o -o acbgrep

test:
	cd src/ahocorasick && gcc aho-corasick.c -c
	cd src && gcc -c -g acbgrep.c
	rm -f test
	gcc ${OPT} ${SRCFILES} src/ahocorasick/aho-corasick.o src/acbgrep.o src/test.c -o test

tests:
	dd if=/dev/urandom of=testfile bs=1024k count=5
	./test testfile
	./acbgrep testfile 3031323334 35363738

