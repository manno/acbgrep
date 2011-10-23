#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "ahocorasick/aho-corasick.h"
#include "libac.h"

/**
 * Callback
 */
void on_pattern_found(struct ac_pattern* p, unsigned long pos){
  printf("[*] found pattern %s at %04lu\n", p->hexstring, pos);
}

/*
 * MAIN
 */
#ifndef TEST
int main(int argc, char *argv[]) {
  int fd;

  if(argc < 2) {
    fprintf(stderr, "bigopen\n");
    fprintf(stderr, "usage: %s <file> <hexstring> <hexstring>\n", *argv);
    return 1;
  }

  // open file
  if((fd = open(argv[1], O_RDONLY))< 0) {
    fprintf(stderr, "error opening %s\n", argv[1]);
    exit(1);
  }

  // parse patterns from cmdline
  struct ac_pattern *patterns[BIG_PATTERNSIZE];
  ac_create_patterns(patterns, argc-2, argv+2);

  // init aho machine
  struct ac_table *table = create_aho( patterns );

  // search in file
  ac_search(fd, table, on_pattern_found);

  return 0;
}
#endif
