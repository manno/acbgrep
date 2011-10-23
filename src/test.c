#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "ahocorasick/aho-corasick.h"
#include "libac.h"
#include "libhexstring.h"

// global find these again
struct ac_finding *searches[BIG_PATTERNSIZE];
struct ac_pattern *patterns[BIG_PATTERNSIZE];

void  grab_patterns( int fd, int min_pattern_num, int min_pattern_length ) {

  // find n patterns in file
  unsigned int file_size = lseek(fd, 0, SEEK_END);
  int patterns_wanted = nrandom(20)+ min_pattern_num;
  fprintf( stderr, "[=] testing %d patterns in a %d byte file\n", patterns_wanted, file_size);

  int i = 0;
  unsigned char buffer[BIG_BUFSIZE];
  int pattern_length = 0;
  lseek(fd, 0, SEEK_SET);
  do {
    int ahead = nrandom(file_size)/patterns_wanted;
    if (ahead <= pattern_length)
      ahead = pattern_length+1;
    unsigned int pos = lseek(fd, ahead, SEEK_CUR);
    pattern_length = nrandom(28) + min_pattern_length;
    int len = read(fd, buffer, pattern_length);
    //fprintf( stderr, "[=] read %d bytes\n", (int) len);
    if (len<0)
      break;
    if (len==0) {
      i=patterns_wanted;
      continue;
    }

    // create search pattern
    unsigned char* pattern = malloc( len );
    memcpy(pattern, buffer, len);

    // create hex representation
    char* hexstring = get_hexstring( pattern, len );

    // and our searches
    patterns[i] = ac_pattern_new( pattern, len, hexstring );
    searches[i] = ac_finding_new( patterns[i], pos );
    patterns[i]->id = i;

    fprintf( stderr, "[ ] pattern id%02d (%s len=%d) at %d\n", i,
        patterns[i]->hexstring, (int) len, pos);

    i++;
  } while (i<patterns_wanted);

  patterns[i] = NULL;
  searches[i] = NULL;

}


void check_pattern_found(struct ac_pattern* p, unsigned int pos){
  //printf("[ ] found pattern id%02d (%s) at %04d\n", p->id, p->hexstring, pos);

  struct ac_finding *search = searches[ p->id ];

  if( search->position == pos ) {
    printf("[*] found expected pattern id%02d (%s) at %04d\n", p->id, p->hexstring, pos);
    search->found++;
  }
}

int main(int argc, char *argv[]) {
  int fd;

  if(argc < 1) {
    fprintf(stderr, "big_test\n");
    fprintf(stderr, "usage: %s <file>\n", *argv);
    return 1;
  }

  int min_pattern_num = 1;
  int min_pattern_length = 1;

  // init random
  unsigned int seed = (unsigned  int) time( NULL );
  srandom( seed );

  // open file
  if((fd = open(argv[1], O_RDONLY))< 0) {
    fprintf(stderr, "error opening %s\n", argv[1]);
    exit(1);
  }

  grab_patterns(fd, min_pattern_num, min_pattern_length);

  // init aho machine
  fprintf( stderr, "[=] create table for all patterns\n" );
  struct ac_table *table = create_aho( patterns );

  // reset fd
  lseek(fd, 0, SEEK_SET);

  // search in file
  fprintf( stderr, "[=] search file\n");
  search(fd, table, check_pattern_found);

  int i = 0;
  int tainted = 0;
  while( searches[i] != NULL ) {
    if (searches[i]->found != 1) {
      fprintf( stderr, "[!!!] pattern id%02d found %d times.\n", searches[i]->pattern->id, searches[i]->found);
      tainted = 1;
    }
    i++;
  }

  //bug? dumpem all
  if (tainted==1) {
    i=0;
    while( searches[i] != NULL ) {
      fprintf( stderr, "[!] pattern id%02d found %d times: %-70s at %d\n",
          searches[i]->pattern->id,
          searches[i]->found,
          searches[i]->pattern->hexstring,
          searches[i]->position
          );
      i++;
    }
  }

  printf( "[!] ok\n");
  return 0;
}
