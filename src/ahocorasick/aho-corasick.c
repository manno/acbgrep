/**
  implementation of the Aho-Corasick pattern matching algorithm.

  original code was taken from another module by Georgios Portokalidis with his permission.
  updated to fit FFPF1.5 in January of 2006 by Willem de Bruijn

  Fairly Fast Packet Filter - http://ffpf.sourceforge.net/

  Copyright (c), 2003 - 2006 Georgios Portokalidis, Herbert Bos & Willem de Bruijn
  contact info : wdebruij_AT_users.sourceforge.net
  */

#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/slab.h>
#else
#include <stdlib.h>
#include <string.h>
#endif

#include "support/list.h"
#include "aho-corasick.h"

#define bug() printf("(BUG) at %s:%d\n",__FUNCTION__,__LINE__)
#define returnbug(a) do {bug(); return(a);} while (0)

/// free an AC table from a given startnode (recursively)
static void ac_free(struct ac_state *state)
{
        int i;

        for(i = 0; i < AHO_CORASICK_CHARACTERS ;i++)
                if ( state->next[i] )
                        ac_free(state->next[i]);
        if (state->output)
                ac_pattern_delete(state->output);
        free(state);
}

/// initialize the empty-table
void ac_init(struct ac_table *g)
{
        g->zerostate = NULL;
        g->idcounter = 0;
        g->patterncounter = 0;
}

/// free an entire AC table
void ac_destroy(struct ac_table *in)
{
        int i;

        for(i = 0; i < AHO_CORASICK_CHARACTERS ;i++)
                if ( in->zerostate->next[i] && in->zerostate->next[i]->id > 0 ){
                        ac_free(in->zerostate->next[i]);
                        in->zerostate->next[i] = NULL;
                }
        free(in->zerostate);
}

int ac_maketree(struct ac_table *g)
{
        struct list *list = NULL;
        struct ac_state *state,*s, *cur;
        int i;

        // Set all NULL transition of 0 state to point to itself
        for(i = 0; i < AHO_CORASICK_CHARACTERS ;i++){
                if ( !g->zerostate->next[i] )
                        g->zerostate->next[i] = g->zerostate;
                else{
                        list = list_append(list, g->zerostate->next[i]);
                        g->zerostate->next[i]->fail = g->zerostate;
                }
        }

        // Set fail() for depth > 0
        while(list){
                cur = (struct ac_state *) list->id;
                for(i = 0; i < AHO_CORASICK_CHARACTERS ;i++){
                        s = cur->next[i];
                        if (s){
                                list = list_append(list,s);
                                state = cur->fail;
                                while( !state->next[i] )
                                        state = state->fail;
                                s->fail = state->next[i];
                        }
                        // Join outputs missing
                }
                list = list_pop(list);
        }

        list_destroy(list);
        return 0;
}

int ac_addpattern(struct ac_table *g, struct ac_pattern* pattern)
{
        struct ac_state *state, *next = NULL;
        int j, done = 0;

        unsigned char *string = pattern->p;
        int slen = pattern->len;

        if ( !g->zerostate ){
                g->zerostate = malloc(sizeof(struct ac_state));
                if (!g->zerostate)
                        returnbug(-1);
                g->idcounter = 1;
                g->zerostate->id = 0;
                g->zerostate->depth = 0;
                g->zerostate->output = NULL;
                memset(g->zerostate->next, 0, AHO_CORASICK_CHARACTERS * sizeof(struct ac_state *));
        }

        // as long as next already exists follow them
        j = 0;
        state = g->zerostate;
        while( !done && (next = state->next[*(string+j)]) != NULL ){
                state = next;
                if ( j == slen )
                        done = 1;
                j++;
        }

        if ( !done ){	// not done yet
                while( j < slen )
                {
                        // Create new state
                        next = malloc(sizeof(struct ac_state));
                        if ( !next )
                                returnbug(-1);
                        next->id = g->idcounter++;
                        next->depth = state->depth + 1;
                        next->output = NULL;
                        memset(next->next, 0, AHO_CORASICK_CHARACTERS * sizeof(struct ac_state *));

                        state->next[*(string+j)] = next;
                        state = next;
                        j++;
                }
        }

        if (!state->output){	// add the leaf (why?)
                state->output = pattern;

                g->patterncounter++;
                //printf("AhoCorasick: added %dth pattern %s, the DFA now consists of %d states\n",
                //		g->patterncounter, string, g->idcounter);
        }

        return 0;
}

unsigned int ac_pattern_id = 0;

struct ac_pattern*
ac_pattern_new(void* mem, size_t len, char* hexstring) {
        struct ac_pattern* p = malloc(sizeof(*p));
        p->p = malloc(len);
        memcpy(p->p, mem, len);
        p->len = len;
        p->id = ac_pattern_id++;
        p->hexstring = strdup(hexstring);
        return p;
}

void
ac_pattern_delete(struct ac_pattern* p){
        free(p->p);
        free(p->hexstring);
        free(p);
}

struct ac_search_context*
ac_search_context_new(struct ac_table* g, ac_pattern_found on_found ) {
        struct ac_search_context* ctx = malloc(sizeof(*ctx));
        ctx->file_offset = 0;
        ctx->g = g;
        ctx->state = g->zerostate;
        ctx->on_found = on_found;
        return ctx;
}

#ifdef DEBUG
char* get_ts(int ts) {
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

void dump_state(struct ac_search_context* ctx, struct ac_state* state, int ts ) {
        char* tabs = get_ts(ts);
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

void
ac_buffer_findall(struct ac_search_context* ctx, unsigned char* buffer, int len) {
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
