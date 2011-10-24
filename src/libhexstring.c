#include <stdio.h>
#include <stdlib.h>
#include "libhexstring.h"

char* get_hexstring( unsigned char* pattern, int len ) {
    char* result = malloc(len*2+1);
    int j;
    for(j=0; j<len; j++)
      sprintf( result+j*2, "%02x", pattern[j] );
    result[j*2] = '\0';
    return result;
}

int nrand( int n ) {
  return rand()/(int)(((unsigned)RAND_MAX + 1) / n);
}

unsigned long nrandom( unsigned long n ) {
  return (random() % n);
}

/*
 * PATTERN related stuff
 */
int ascii2hex( const char c )
{
  // taken from bgrep.c (https://raw.github.com/tmbinc/bgrep/master/bgrep.c)
  if (c < '0')
    return -1;
  else if (c <= '9')
    return c - '0';
  else if (c < 'A')
    return -1;
  else if (c <= 'F')
    return c - 'A' + 10;
  else if (c < 'a')
    return -1;
  else if (c <= 'f')
    return c - 'a' + 10;
  else
    return -1;
}

