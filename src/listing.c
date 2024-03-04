#define LISTING__C

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <time.h>                     // [RLA] asctime(), localtime(), struct tm, etc...

#include "macro11.h"                  // [RLA] needed for VERSIONSTR
#include "listing.h"                   /* my own definitions */

#include "util.h"
#include "assemble_globals.h"


/* GLOBAL VARIABLES */

int             list_md = 1;    /* option to list macro/rept definition = yes */

int             list_me = 1;    /* option to list macro/rept expansion = yes */

int             list_bex = 1;   /* option to show binary */

int             list_level = 1; /* Listing control level.  .LIST
                                   increments; .NLIST decrements */

int             force_new_page  = 1;    // [RLA] TRUE to force a new listing page
int             lines_per_page  = 60;   // [RLA] lines per listing page
int             lines_this_page = 0;    // [RLA] lines printed on this page
static int      current_page = 0;       // [RLA] current listing page number
char program_title[STREAM_BUFFER_SIZE]    = ""; // [RLA] title string from .TITLE 
char current_subtitle[STREAM_BUFFER_SIZE] = ""; // [RLA] current .SBTTL string 

static char    *listline;       /* Source lines */

static char    *binline;        /* for octal expansion */

FILE           *lstfile = NULL;

int             list_pass_0 = 0;/* Also list what happens during the first pass */



/* do_list returns TRUE if listing is enabled. */

static int dolist(
    void)
{
    int             ok = lstfile != NULL &&
                         (pass > 0 || list_pass_0) &&
                         list_level > 0;

    return ok;
}

/* list_source saves a text line for later listing by list_flush */

void list_source(
    STREAM *str,
    char *cp)
{
    if (dolist()) {
        int             len = strcspn(cp, "\n");

        /* Save the line text away for later... */
        if (listline)
            free(listline);
        listline = memcheck(malloc(len + 1));
        memcpy(listline, cp, len);
        listline[len] = 0;

        if (!binline)
            binline = memcheck(malloc(sizeof(LSTFORMAT) + 16));

        sprintf(binline, "%*s%*d", (int)SIZEOF_MEMBER(LSTFORMAT, flag), "", (int)SIZEOF_MEMBER(LSTFORMAT, line_number),
                str->line);
    }
}

// [RLA]   Return the current system date and time as an ASCIZ
// [RLA] string.  Note that the result is returned in a static
// [RLA] buffer!
char *get_date_and_time (void)
{
  time_t tm;  struct tm *ptm;
  time(&tm);  ptm = localtime(&tm);
  char *months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                      "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  static char buf[128];
  sprintf(buf, "%02d-%3s-%02d %02d:%02d",
    ptm->tm_mday, months[ptm->tm_mon], (ptm->tm_year % 100),
    ptm->tm_hour, ptm->tm_min);
  return buf;
}

// [RLA] Start an new listing page ...
void new_listing_page (void)
{
  if (current_page > 0)  fprintf(lstfile, "\f");
  fprintf(lstfile, "%-89s MACRO11 V%-15s Page %3d\n", program_title, BASE_VERSION, ++current_page);
  fprintf(lstfile, "%-84s %38s\n", current_subtitle, get_date_and_time());
  fprintf(lstfile, "\n");
  lines_this_page = 0;  force_new_page = 0;
}

/* list_flush produces a buffered list line. */

void list_flush(
    void)
{
    if (dolist()) {
        padto(binline, offsetof(LSTFORMAT, source));
        if (force_new_page || (++lines_this_page > lines_per_page)) new_listing_page();
        fputs(binline, lstfile);
        fputs(listline, lstfile);
        fputc('\n', lstfile);
        listline[0] = 0;
        binline[0] = 0;
    }
}

/* list_fit checks to see if a word will fit in the current listing
   line.  If not, it flushes and prepares another line. */

static void list_fit(
    STREAM *str,
    unsigned addr)
{
    size_t          col1 = offsetof(LSTFORMAT, source);
    size_t          col2 = offsetof(LSTFORMAT, pc);

    if (strlen(binline) >= col1) {
        list_flush();
        listline[0] = 0;
        binline[0] = 0;
        sprintf(binline, "%*s %6.6o", (int)offsetof(LSTFORMAT, pc), "", addr);
        padto(binline, offsetof(LSTFORMAT, words));
    } else if (strlen(binline) <= col2) {
        sprintf(binline, "%*s%*d %6.6o", (int)SIZEOF_MEMBER(LSTFORMAT, flag), "",
                (int)SIZEOF_MEMBER(LSTFORMAT, line_number), str->line, addr);
        padto(binline, offsetof(LSTFORMAT, words));
    }
}

/* list_value is used to show a computed value */

void list_value(
    STREAM *str,
    unsigned word)
{
    if (dolist()) {
        /* Print the value and go */
        binline[0] = 0;
        sprintf(binline, "%*s%*d        %6.6o", (int)SIZEOF_MEMBER(LSTFORMAT, flag), "",
                (int)SIZEOF_MEMBER(LSTFORMAT, line_number), str->line, word & 0177777);
    }
}

/* Print a word to the listing file */

void list_word(
    STREAM *str,
    unsigned addr,
    unsigned value,
    int size,
    char *flags)
{
    if (dolist()) {
        list_fit(str, addr);
        if (size == 1)
            sprintf(binline + strlen(binline), "   %3.3o%1.1s ", value & 0377, flags);
        else
            sprintf(binline + strlen(binline), "%6.6o%1.1s ", value & 0177777, flags);
    }
}


/* Print just a line with the address to the listing file */

void list_location(
    STREAM *str,
    unsigned addr)
{
    if (dolist()) {
        list_fit(str, addr);
    }
}



/* reports errors */
void report(
    STREAM *str,
    char *fmt,
    ...)
{
    va_list         ap;
    char           *name = "**";
    int             line = 0;

    if (!pass && list_pass_0 < 2)
        return;                        /* Don't report now. */

    if (str) {
        name = str->name;
        line = str->line;
    }

    fprintf(stderr, "%s:%d: ***ERROR ", name, line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (lstfile) {
        fprintf(lstfile, "%s:%d: ***ERROR ", name, line);
        va_start(ap, fmt);
        vfprintf(lstfile, fmt, ap);
        va_end(ap);
    }
}
