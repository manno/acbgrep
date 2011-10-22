#include <stdio.h>
#include <stdlib.h>
#include "ahocorasick/aho-corasick.h"
#include "libac.h"
#include "libhexstring.h"

/**
 * search
 */
void search(int fd, struct ac_table* table, ac_pattern_found on_pattern_found) {
  unsigned char buffer[BIG_BUFSIZE];
  struct ac_search_context *ctx =  ac_search_context_new( table, on_pattern_found );
  int r;

  // iterate all buffers in file
  while( r = read(fd, buffer, BIG_BUFSIZE) ) {
    /*
    register int offs=0;
    // iterate all sequential findings in buffer
    do {
      offs = ac_buffer_search( ctx, buffer, offs, r );
    } while (offs >= 0 && offs<=r);
    */
    ac_buffer_findall( ctx, buffer, r );
    ctx->file_offset += r;
  }
}

void create_patterns( struct ac_pattern* patterns[], int argc, char *argv[]) {
  // parse patterns, last pointer is null
  int i = 0;
  while( i < argc && i < BIG_PATTERNSIZE ) {
    patterns[i] = hexstring2byte( argv[i] );
    fprintf( stderr, "[ ] pattern %01d = %s\n", i, patterns[i]->hexstring);
    i++;
  }
  patterns[i] = NULL;
  fprintf( stderr, "[=] %d patterns total\n", i);
}

/**
 * create_aho
 */
struct ac_table* create_aho( struct ac_pattern* patterns[] ) {
  struct ac_table *table = malloc(sizeof(struct ac_table));
  if (!table)
    fprintf( stderr, "[!] failed reserving mem for table" );
  ac_init(table);

  int i = 0;
  while( patterns[i] != NULL ) {
    if (ac_addpattern(table, patterns[i]) < 0)
      fprintf( stderr, "[!] failed adding exp %d\n", i );
    i++;
  }

  if ( i==0 ) {
    fprintf( stderr, "[!] no patterns found\n" );
    exit(1);
  }

  if ( ac_maketree(table) < 0 ) {
    fprintf( stderr, "[!] failed making a-tree\n" );
    exit(1);
  }

 return table;
}

struct ac_pattern* hexstring2byte( char* hexstring ) {
  char  *hexbackup = hexstring;

  unsigned char value[BIG_PATTERNSIZE];
  int len = 0;
  while( *hexstring && hexstring[1] && len < BIG_PATTERNSIZE ) {
    int v0 = ascii2hex(*hexstring++);
    int v1 = ascii2hex(*hexstring++);
    if (v0 == -1 || v1 == -1) {
      fprintf( stderr, "invalid hex string\n" );
      return NULL;
    }
    value[len] = (v0 << 4) | v1;
    len++;
  }
  return ac_pattern_new( value, len, hexbackup );
}

struct  big_searches* big_searches_new(struct ac_pattern* pattern, unsigned int position) {
  struct big_searches *bs = malloc( sizeof(*bs) );
  // expected pat
  bs->pattern = pattern;
  // expected pos
  bs->position = position;
  bs->found = 0;
  return bs;
}

