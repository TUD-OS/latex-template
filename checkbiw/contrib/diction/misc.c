/* Notes */ /*{{{C}}}*//*{{{*/
/*

This file is free software; as a special exception the author gives
unlimited permission to copy and/or distribute it, with or without
modifications, as long as this notice is preserved.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/
/*}}}*/
/* #includes *//*{{{*/
#ifndef NO_POSIX_SOURCE
#undef  _POSIX_SOURCE
#define _POSIX_SOURCE 1
#undef  _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 2
#endif

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

#include "config.h"

#include "misc.h"
/*}}}*/

#ifdef BROKEN_REALLOC
/* myrealloc   -- ANSI conforming realloc() */ /*{{{*/
#undef realloc
void *myrealloc(void *p, size_t n)
{
  return (p==(void*)0 ? malloc(n) : realloc(p,n));
}
/*}}}*/
#endif
#ifndef HAVE_STRERROR
/* strerror    -- ANSI strerror */ /*{{{*/
extern int sys_nerr;
extern char *sys_errlist[];

char *strerror(int errno)
{
  assert(errno>=0);
  assert(errno<sys_nerr);
  return sys_errlist[errno];
}
/*}}}*/
#endif
