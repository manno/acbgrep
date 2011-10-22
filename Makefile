SRCFILES = src/libac.c src/libhexstring.c
#DEFS = -DDEBUG
#OPT  = -g

all: binsearch test

binsearch:
	cd src/ahocorasick && gcc ${DEFS} aho-corasick.c -c
	rm -f bingrep
	gcc ${OPT} ${DEFS} -DBINSEARCH src/bingrep.c ${SRCFILES} src/ahocorasick/aho-corasick.o -o bingrep
	
test:
	cd src/ahocorasick && gcc aho-corasick.c -c
	cd src && gcc -c -g bingrep.c
	rm -f test
	gcc ${OPT} ${SRCFILES} src/ahocorasick/aho-corasick.o src/bingrep.o src/test.c -o test

tests:
	dd if=/dev/urandom of=testfile bs=1024k count=5
	./test testfile
	./bingrep testfile 3031323334 35363738

