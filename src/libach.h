// maximum number and length of search patterns
#define ACH_PATTERNSIZE 255
#define ACH_BUFSIZE BUFSIZ

#ifdef TEST
struct ach_finding {
        struct ac_pattern* pattern;
        unsigned long position;
        int found;
};

struct  ach_finding* ach_finding_new(struct ac_pattern*, unsigned long );
#endif

typedef void(*ach_pattern_found)(struct ac_pattern*, unsigned long pos);

struct ach_search_context {
        unsigned long file_offset;
        struct ac_state *state;
        struct ac_table *g;
        ach_pattern_found on_found;
};

struct ach_search_context* ach_search_context_new(struct ac_table*, ach_pattern_found);

void ach_create_patterns( struct ac_pattern*[], int, char *[]);

struct ac_pattern* ach_hexstring2pattern( char* );

struct ac_table* ach_create_aho( struct ac_pattern*[] );

void ach_search(int, struct ac_table*, ach_pattern_found);

void ach_buffer_findall(struct ach_search_context*, unsigned char*, int);

