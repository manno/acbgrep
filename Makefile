all: big bigt
SRCFILES = src/libac.c src/libhexstring.c
DEFS = -DNODEBUG

big:
	cd src/ahocorasick && gcc ${DEFS} aho-corasick.c -c
	rm -f bingrep
	gcc -g ${DEFS} -Dbingrep src/bingrep.c ${SRCFILES} src/ahocorasick/aho-corasick.o -o bingrep
	
test:
	cd src/ahocorasick && gcc aho-corasick.c -c
	cd src && gcc -c -g bingrep.c
	rm -f test
	gcc ${OPT} ${SRCFILES} src/ahocorasick/aho-corasick.o src/bingrep.o src/test.c -o test
