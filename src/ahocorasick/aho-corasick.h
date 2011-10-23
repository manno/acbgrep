/**
  implementation of the Aho-Corasick pattern matching algorithm.

  original code was taken from another module by Georgios Portokalidis with his permission.
  updated to fit FFPF1.5 in January of 2006 by Willem de Bruijn

  Fairly Fast Packet Filter - http://ffpf.sourceforge.net/

  Copyright (c), 2003 - 2006 Georgios Portokalidis, Herbert Bos & Willem de Bruijn
  contact info : wdebruij_AT_users.sourceforge.net
  */

#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#define AHO_CORASICK_CHARACTERS 256

struct ac_pattern {
        unsigned char* p;
        char* hexstring;
        size_t len;
        unsigned int id;
};

struct ac_state {
        unsigned int id;
        unsigned int depth;
        struct ac_pattern *output;
        struct ac_state *fail;
        struct ac_state *next[AHO_CORASICK_CHARACTERS];
};

struct ac_table {
        unsigned int idcounter;
        unsigned int patterncounter;
        struct ac_state *zerostate;
};

struct ac_pattern* ac_pattern_new(void*, size_t, char*);
void ac_pattern_delete(struct ac_pattern*);
void ac_init(struct ac_table *);
int ac_addpattern(struct ac_table *, struct ac_pattern* );
int ac_maketree(struct ac_table *);

typedef void(*ac_pattern_found)(struct ac_pattern*, unsigned long pos);
struct ac_search_context* ac_search_context_new(struct ac_table*, ac_pattern_found);

#define AC_FOUND_ERROR -1

struct ac_search_context {
        unsigned long file_offset;
        struct ac_state *state;
        struct ac_table *g;
        ac_pattern_found on_found;
};

void ac_destroy(struct ac_table *);
void ac_buffer_findall(struct ac_search_context*, unsigned char*, int);

#endif

