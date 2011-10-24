// maximum number and length of search patterns
#define BIG_PATTERNSIZE 255
#define BIG_BUFSIZE BUFSIZ

struct ac_finding {
        struct ac_pattern* pattern;
        unsigned long position;
        int found;
};


void ac_search(int, struct ac_table*, ac_pattern_found);
struct ac_table* create_aho( struct ac_pattern*[] );
void ac_create_patterns( struct ac_pattern*[], int, char *[]);
struct ac_pattern* hexstring2byte( char* );
struct  ac_finding* ac_finding_new(struct ac_pattern*, unsigned long );
