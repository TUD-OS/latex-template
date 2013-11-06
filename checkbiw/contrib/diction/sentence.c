/* Notes */ /*{{{C}}}*//*{{{*/
/*

This program is GNU software, copyright 1997, 1998, 1999, 2000, 2001
Michael Haardt <michael@moria.de>.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/
/*}}}*/
/* #includes */ /*{{{*/
#undef  _POSIX_SOURCE
#define _POSIX_SOURCE   1
#undef  _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 2

#include "config.h"

#include <sys/types.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#ifdef HAVE_GETTEXT
#include <libintl.h>
#define _(String) gettext(String)
#else
#define _(String) String
#endif
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "sentence.h"
/*}}}*/

static const char *abbreviations_de[]= /*{{{*/
{
  "Dr",
  "bzw",
  "etc",
  "sog",
  "usw",
  (const char*)0
};
/*}}}*/
static const char *abbreviations_en[]= /*{{{*/
{
  "ch",
  "Ch",
  "ckts",
  "dB",
  "Dept",
  "dept",
  "Depts",
  "depts",
  "Dr",
  "Drs",
  "Eq",
  "eq",
  "etc",
  "et al",
  "Fig",
  "fig",
  "Figs",
  "figs",
  "ft",
  "0 in",
  "1 in",
  "2 in",
  "3 in",
  "4 in",
  "5 in",
  "6 in",
  "7 in",
  "8 in",
  "9 in",
  "Inc",
  "Jr",
  "jr",
  "mi",
  "Mr",
  "Mrs",
  "Ms",
  "No",
  "no",
  "Nos",
  "nos",
  "Ph",
  "Ref",
  "ref",
  "Refs",
  "refs",
  "St",
  "vs",
  "yr",
  (const char*)0
};
/*}}}*/
static const char *abbreviations_none[]= /*{{{*/
{
  (const char*)0
};
/*}}}*/
static const char **abbreviations;

static int endingInAbbrev(const char *s, size_t length, const char *lang) /*{{{*/
{
  const char **abbrev=abbreviations;
  size_t aLength;

  if (!isalpha(s[length-1])) return 0;
  if (endingInPossesiveS(s,length)) return 0;
  while (*abbrev!=(const char*)0)
  {
    if ((aLength=strlen(*abbrev))<length)
    {
      if (!isalpha(s[length-2])) return 1;
      if (!isalpha(s[length-aLength-1]) && strncmp(s+length-aLength,*abbrev,aLength)==0) return 1;
    }
    else
    {
      if (length==1) return 1;
      if (aLength==length && strncmp(s,*abbrev,aLength)==0) return 1;
    }      
    ++abbrev;
  }
  return 0;
}
/*}}}*/

int endingInPossesiveS(const char *s, size_t length) /*{{{*/
{
  return (abbreviations==abbreviations_en && length>=3 && strncmp(s+length-2,"\'s",2)==0);
}
/*}}}*/
void sentence(const char *cmd, FILE *in, const char *file, void (*process)(const char *, size_t, const char *, int), const char *lang) /*{{{*/
{
  /* variables */ /*{{{*/
  int voc,oc,c;
  char *sent=malloc(128);
  size_t length=0,capacity=128;
  int inWhiteSpace=0;
  int inParagraph=0;
  int ellipsis=0;
  int line=1,beginLine=1;
  int err;
  regex_t hashLine;
  char filebuf[_POSIX_PATH_MAX+1];
  /*}}}*/

  if (strncmp(lang,"en",2)==0) abbreviations=abbreviations_en;
  else if (strncmp(lang,"C",1)==0) abbreviations=abbreviations_en;
  else if (strncmp(lang,"de",2)==0) abbreviations=abbreviations_de;
  else abbreviations=abbreviations_none;
  /* compile #line number "file" regular expression */ /*{{{*/
  if ((err=regcomp(&hashLine,"^[ \t]*line[ \t]*\\([0-9][0-9]*\\)[ \t]*\"\\([^\"]*\\)\"",0)))
  {
    char buf[256];
    size_t len=regerror(err,&hashLine,buf,sizeof(buf)-1);
    buf[len]='\0';
    fprintf(stderr,_("%s: internal error, compiling a regular expression failed (%s).\n"),cmd,buf);
    exit(2);
  }
  /*}}}*/
  voc='\n';
  c=getc(in);
  while ((oc=c)!=EOF)
  {
    c=getc(in);
    if (oc=='\n') ++line;
    if (voc=='\n' && oc=='#') /* process cpp style #line, continue */ /*{{{*/
    {
      char buf[_POSIX_PATH_MAX+20];
      regmatch_t found[3];

      buf[0]=c; buf[1]='\0';
      (void)fgets(buf+1,sizeof(buf)-1,in);
      if (regexec(&hashLine,buf,3,found,0)==0) /* #line */ /*{{{*/
      {
        size_t len;

        line=strtol(buf+found[1].rm_so,(char**)0,10)-1;
        len=found[2].rm_eo-found[2].rm_so;
        if (len>_POSIX_PATH_MAX) len=_POSIX_PATH_MAX;
        strncpy(filebuf,buf+found[2].rm_so,len);
        filebuf[len]='\0';
        file=filebuf;
      }
      /*}}}*/
      c='\n';
      continue;
    }
    /*}}}*/
    if (length)
    {
      if (length>=(capacity-1) && (sent=realloc(sent,capacity*=2))==(char*)0)
      {
        fprintf(stderr,_("%s: increasing sentence buffer failed: %s\n"),cmd,strerror(errno));
        exit(2);
      }
      if (isspace(oc))
      {
        if (!inWhiteSpace)
        {
          sent[length++]=' ';
          inWhiteSpace=1;
        }
      }
      else
      {
        sent[length++]=oc;
        if
        (
          (length==3 && strncmp(sent+length-3,"...",3)==0 && (c==EOF || isspace(c)))
          || (length>=4 && strncmp(sent+length-4," ...",4)==0 && (c==EOF || isspace(c)))
        )
        {
          /* omission ellipsis */
          inWhiteSpace=0;
        }
        else if (length>=4 && !isspace(sent[length-4]) && strncmp(sent+length-3,"...",3)==0 && (c==EOF || isspace(c)))
        {
          /* beginning ellipsis */
          char foo;

          foo=sent[length-4];
          sent[length-4]='\0';
          process(sent,length-4,file,beginLine);
          sent[length-4]=foo;
          memmove(sent,sent-4,4);
          length=4;
          inParagraph=0;
          inWhiteSpace=0;
          beginLine=line;
        }
        else if (length>=4 && strncmp(sent+length-4,"...",3)==0 && (c==EOF || isspace(c)))
        {
          /* ending ellipsis */
          if (inWhiteSpace) --length;
          sent[length]='\0';
          process(sent,length,file,beginLine);
          length=0;
        }
        else if ((oc=='.' || oc==':' || oc=='!' || oc=='?') && (c==EOF || isspace(c)) && (oc!='.' || !endingInAbbrev(sent,length,lang)))
        {
          /* end of sentence */
          if (inWhiteSpace) --length;
          sent[length]='\0';
          process(sent,length,file,beginLine);
          length=0;
        }
        else
        {
          /* just a regular character */
          inWhiteSpace=0;
        }
      }
    }
    else if (isupper(oc))
    {
      inParagraph=0;
      sent[length++]=oc;
      inWhiteSpace=0;
      beginLine=line;
    }
    else if (!inParagraph && oc=='\n' && c==oc)
    {
      process("",0,file,line);
      inParagraph=1;
    }
    voc=oc;
  }
  if (!inParagraph) process("",0,file,line);
  regfree(&hashLine);
}
/*}}}*/
