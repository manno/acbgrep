// maximum number and length of search patterns
#define BIG_PATTERNSIZE 255
#define BIG_BUFSIZE BUFSIZ

struct big_searches {
  struct ac_pattern* pattern;
  unsigned int position;
  int found;
};


void search(int, struct ac_table*, ac_pattern_found);
struct ac_table* create_aho( struct ac_pattern*[] );
void create_patterns( struct ac_pattern*[], int, char *[]);
struct ac_pattern* hexstring2byte( char* );
struct  big_searches* big_searches_new(struct ac_pattern*, unsigned int );
