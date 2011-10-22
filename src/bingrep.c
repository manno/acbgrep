#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "ahocorasick/aho-corasick.h"
#include "libac.h"

/**
 * Callback
 */
void on_pattern_found(struct ac_pattern* p, unsigned int pos){
  printf("[*] found pattern %s at %04d\n", p->hexstring, pos);
  // expect 0, 1, 3, 11, 12
}

#ifdef BIGOPEN
/*
 * MAIN
 */
int main(int argc, char *argv[]) {
  int fd;

  if(argc < 2) {
    fprintf(stderr, "bigopen\n");
    fprintf(stderr, "usage: %s <file> <hex> <hex>\n", *argv);
    return 1;
  }

  // open file
  if((fd = open(argv[1], O_RDONLY))< 0) {
    fprintf(stderr, "error opening %s\n", argv[1]);
    exit(1);
  }

  // parse patterns from cmdline
  struct ac_pattern *patterns[BIG_PATTERNSIZE];
  create_patterns(patterns, argc-2, argv+2);

  // init aho machine
  struct ac_table *table = create_aho( patterns );

  // search in file
  search(fd, table, on_pattern_found);

  return 0;
}
#endif
