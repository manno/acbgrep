/*
 * aho-corasick helper
 */

#include <stdio.h>
#include <stdlib.h>
#include "ahocorasick/aho-corasick.h"
#include "libach.h"
#include "libhexstring.h"

struct ac_pattern* ach_hexstring2byte( char* hexstring ) {
        char  *hexbackup = hexstring;

        unsigned char value[ACH_PATTERNSIZE];
        int len = 0;
        while( *hexstring && hexstring[1] && len < ACH_PATTERNSIZE ) {
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

void ach_create_patterns( struct ac_pattern* patterns[], int argc, char *argv[]) {
        // parse patterns, last pointer is null
        int i = 0;
        while( i < argc && i < ACH_PATTERNSIZE ) {
                patterns[i] = ach_hexstring2byte( argv[i] );
                fprintf( stderr, "[ ] pattern %01d = %s\n", i, patterns[i]->hexstring);
                i++;
        }
        patterns[i] = NULL;
        fprintf( stderr, "[=] %d patterns total\n", i);
}

/**
 * create_aho
 */
struct ac_table* ach_create_aho( struct ac_pattern* patterns[] ) {
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

struct ach_search_context*
ach_search_context_new(struct ac_table* g, ach_pattern_found on_found ) {
        struct ach_search_context* ctx = malloc(sizeof(*ctx));
        ctx->file_offset = 0;
        ctx->g = g;
        ctx->state = g->zerostate;
        ctx->on_found = on_found;
        return ctx;
}

#ifdef TEST
struct  ach_finding* ach_finding_new(struct ac_pattern* pattern, unsigned long position) {
        struct ach_finding *bs = malloc( sizeof(*bs) );
        // expected pat
        bs->pattern = pattern;
        // expected pos
        bs->position = position;
        bs->found = 0;
        return bs;
}
#endif

#ifdef DEBUG
char* get_tabs(int ts) {
        if(ts==0)
                return "";
        char* result = malloc(ts+1);
        int i=0;
        do {
                result[i] = ' ';
                i++;
        } while ( i < ts );
        result[i] = '\0';
        return result;
}

void dump_state(struct ach_search_context* ctx, struct ac_state* state, int ts ) {
        char* tabs = get_tabs(ts);
        if (ts>40)
                return;
        if (!state) {
                printf("%s<null/>\n", tabs);
                return;
        }
        if (state==ctx->g->zerostate && ts>0) {
                printf("%s<zerostate/>\n", tabs);
                return;
        }

        printf("%s<id%02d text='%s' depth='%d'>\n", tabs,
                        state->id, state->output ? state->output->hexstring : "NT", state->depth);
        printf( "  %s<fail of='%d'>\n", tabs, state->id );
        dump_state(ctx, state->fail,ts+4);
        printf("  %s</fail>\n", tabs);

        int i = 0;
        for( i=0;i<AHO_CORASICK_CHARACTERS;i++) {
                //state->next == ctx->g->zerostate <=> state->next->fail == null
                if( state->next[i] && state->next[i] != ctx->g->zerostate ) {
                        printf( "  %s<next of='%d' for='0x%02x'>\n", tabs, state->id, i );
                        dump_state(ctx, state->next[i],ts+4);
                        printf( "  %s</next>\n", tabs);
                }
        }
        printf("%s</id%02d>\n", tabs, state->id);
}
#endif

/**
 * search
 */
void ach_search(int fd, struct ac_table* table, ach_pattern_found on_pattern_found) {
        unsigned char buffer[ACH_BUFSIZE];
        struct ach_search_context *ctx =  ach_search_context_new( table, on_pattern_found );
        int r;

        // iterate all buffers in file
        while( r = read(fd, buffer, ACH_BUFSIZE) ) {
                ach_buffer_findall( ctx, buffer, r );
                ctx->file_offset += r;
        }
}

void
ach_buffer_findall(struct ach_search_context* ctx, unsigned char* buffer, int len) {
        register struct ac_state *nextstate;
        int j=0;
        do {
                nextstate = ctx->state->next[*(buffer+j)];
                if(nextstate == NULL){
                        ctx->state = ctx->state->fail;
                } else{
                        ctx->state = nextstate;
                }

                // we don't want to check the root node on a possible pattern start
                if (ctx->state == ctx->g->zerostate) {
                        nextstate = ctx->state->next[*(buffer+j)];
                        // if it's not a new pattern, try next char on next loop
                        ctx->state = nextstate != NULL ? nextstate : ctx->g->zerostate;
                }

#ifdef DEBUG
                dump_state(ctx, ctx->state, 0);
#endif

                // is this a node with content, a match?
                if(ctx->state->output != NULL) {
                        int found = j - ctx->state->depth + 1;
                        unsigned long pos = ctx->file_offset + found;
                        ctx->on_found( ctx->state->output, pos );
                }

                j++;

        } while (j<len);
}
