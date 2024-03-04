
#ifndef LISTING__H
#define LISTING__H

#include "stream2.h"

/*
    format of a listing line
    Interestingly, no instances of this struct are ever created.
    It lives to be a way to layout the format of a list line.
    I wonder if I should have bothered.
*/

typedef struct lstformat {
    char            flag[2];    /* Error flags */
    char            line_number[6];     /* Line number */
    char            pc[8];      /* Location */
    char            words[8][3];        /* three instruction words */
    char            source[1];  /* source line */
} LSTFORMAT;


/* GLOBAL VARIABLES */
#ifndef  LISTING__C
extern int      list_md;        /* option to list macro/rept definition = yes */

extern int      list_me;        /* option to list macro/rept expansion = yes */

extern int      list_bex;       /* option to show binary */

extern int      list_level;     /* Listing control level.  .LIST
                                   increments; .NLIST decrements */

extern int      force_new_page; // [RLA] TRUE to force a new listing page
extern int      lines_per_page; // [RLA] lines per listing page
extern int      lines_this_page;// [RLA] lines printed on this page
extern char     program_title[];   // [RLA] title string from .TITLE 
extern char     current_subtitle[];// [RLA] subtitle string from .SBTTL

extern FILE    *lstfile;

extern int      list_pass_0;    /* Also list what happens during the first pass */

#endif

void new_listing_page(void);  // [RLA]

void            list_word(
    STREAM *str,
    unsigned addr,
    unsigned value,
    int size,
    char *flags);

void            list_value(
    STREAM *str,
    unsigned word);

void            list_location(
    STREAM *str,
    unsigned word);

void            list_source(
    STREAM *str,
    char *cp);

void            list_flush(
    void);

void            report(
    STREAM *str,
    char *fmt,
    ...);


#endif
