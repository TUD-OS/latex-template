/* Notes */ /*{{{C}}}*//*{{{*/
/*

This program is GNU software, copyright 1997-2004
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
#include <unistd.h>

#include "getopt.h"
#include "misc.h"
#include "sentence.h"
/*}}}*/
/* types */ /*{{{*/
struct badPhrase
{
  char *phrase;
  regex_t phrase_r;
  char *suggest;
};
/*}}}*/

static int doubleWords=1;
static char phraseLanguage[32];
static struct badPhrase *badPhrases=(struct badPhrase *)0;
static int badPhraseCapacity=0;
static int badPhraseSize=0;
static int sentences,hits;

static void loadPhrases(const char *file) /*{{{*/
{
  FILE *fp;
  char ln[1024];
  char *tab;
  size_t l;
  int fix,j;

  if ((fp=fopen(file,"r"))==(FILE*)0)
  {
    fprintf(stderr,_("diction: Opening `%s' failed (%s).\n"),file,strerror(errno));
    exit(1);
  }
  while (fgets(ln,sizeof(ln),fp))
  {
    l=strlen(ln);
    if (l && ln[l-1]=='\n') ln[--l]='\0';
    if (ln[0])
    {
      int err;

      if (badPhraseSize==badPhraseCapacity) /* enlarge capacity */ /*{{{*/
      {
        if ((badPhrases=realloc(badPhrases,(badPhraseCapacity=3*(badPhraseCapacity+32))*sizeof(struct badPhrase)))==(struct badPhrase*)0)
        {
          fprintf(stderr,_("diction: out of memory.\n"));
          exit(2);
        }
      }
      /*}}}*/
      if ((tab=strchr(ln,'\t')))
      {
        *tab='\0';
        ++tab;
      }
      if ((badPhrases[badPhraseSize].phrase=malloc(strlen(ln)+1))==(char*)0)
      {
        fprintf(stderr,_("diction: out of memory.\n"));
        exit(2);
      }
      strcpy(badPhrases[badPhraseSize].phrase,ln);
#if 0
      if ((err=regcomp(&badPhrases[badPhraseSize].phrase_r,ln,REG_EXTENDED))!=0)
      {
        char errmsg[1024];

        regerror(err,&badPhrases[badPhraseSize].phrase_r,errmsg,sizeof(errmsg));
        fprintf(stderr,_("diction: Compiling regular expression `%s' failed (%s).\n"),ln,errmsg);
        exit(2);
      }
#endif
      if (tab)
      {
        if ((badPhrases[badPhraseSize].suggest=malloc(strlen(tab)+1))==(char*)0)
        {
          fprintf(stderr,_("diction: out of memory.\n"));
          exit(2);
        }
        strcpy(badPhrases[badPhraseSize].suggest,tab);
      }
      else badPhrases[badPhraseSize].suggest=(char*)0;
      ++badPhraseSize;
    }
  }
  /* resolve =phrase explainations */ /*{{{*/
  for (fix=0; fix<badPhraseSize; ++fix)
  {
    if (badPhrases[fix].suggest && *badPhrases[fix].suggest=='=')
    {
      for (j=0; j<badPhraseSize; ++j)
      {
        if (j!=fix && strcmp(badPhrases[j].phrase,badPhrases[fix].suggest+1)==0)
        {
          free(badPhrases[fix].suggest);
          badPhrases[fix].suggest=badPhrases[j].suggest;
          break;
        }
        if (j==badPhraseSize)
        {
          fprintf(stderr,"diction: Warning: Unable to resolve %s.\n",badPhrases[fix].suggest);
        }
      }
    }
  }
  /*}}}*/
}
/*}}}*/
static void diction(const char *sent, size_t length, const char *file, int line) /*{{{*/
{
  const char *lastout=sent;
  const char *s=sent;
  const char *end=sent+length;
  const char *lastWord=(const char*)0;
  int j;

  if (length==0) return;
  while (s<end)
  {
    /* check for bad phrase */ /*{{{*/
    for (j=0; j<badPhraseSize; ++j)
    {
      const struct badPhrase *bp;
      const char *badword,*str;

      bp=&badPhrases[j];
      badword=bp->phrase;
      if (*badword==' ') /* beginning of sentence or word */
      {
        if (s>sent && isalpha(*(s-1))) continue;
        ++badword;
      }
      str=s;
      while ((*badword==tolower(*str) || *badword==*str) && *badword && *str) { ++badword; ++str; }
      if ((*badword=='\0' && !isalpha(*str)) || (*badword=='~' && isalpha(*str)))
      {
        if (bp->suggest && *bp->suggest!='!')
        {
          ++hits;
          if (lastout==sent) printf("%s:%d: ",file,line);
          while (lastout<s) putc(*lastout++,stdout);
          putc('[',stdout);
          while (lastout<str) putc(*lastout++,stdout);
          if (bp->suggest)
          {
            putc(' ',stdout);
            putc('-',stdout);
            putc('>',stdout);
            putc(' ',stdout);
            fputs(bp->suggest,stdout);
          }
          putc(']',stdout);
        }
        s=str-1;
        break;
      }
    }
    /*}}}*/
    /* check for double words */ /*{{{*/
    if (doubleWords)
    {
      const char *badword,*str;

      if (s>sent && !isalpha(*(s-1)))
      {
        /* move back to end of last word */
        badword=s-1;
        while (badword>=sent && !isalpha(*badword)) --badword;
        if (badword>sent)
        {
          /* move back to begin of last word */
          while (badword>=sent && isalpha(*badword)) --badword;
          if (!isalpha(*badword)) ++badword;
          str=s;
          while (*badword==*str && badword<s && isalpha(*str)) { ++badword; ++str; }
          if (badword<s && !isalpha(*badword) && !isalpha(*str))
          {
            if (lastout==sent) printf("%s:%d: ",file,line);
            while (lastout<s) putc(*lastout++,stdout);
            putc('[',stdout);
            while (lastout<str) putc(*lastout++,stdout);
            putc(' ',stdout);
            putc('-',stdout);
            putc('>',stdout);
            putc(' ',stdout);
            fputs(_("Double word."),stdout);
            putc(']',stdout);
            lastWord=s;
            s=str-1;
          }
        }
      }
    }
    /*}}}*/
    ++s;
  }
  ++sentences;
  if (lastout!=sent)
  {
    while (lastout<end) putc(*lastout++,stdout);
    putc('\n',stdout);
    putc('\n',stdout);
  }
}
/*}}}*/
static void print_usage(FILE *handle) /*{{{*/
{
    fputs(_("\
Usage: diction [-d] [-f file [-n|-L language]] [file ...]\n\
       diction [--ignore-double-words]\n\
               [--file file [--no-default-file|--language]] [file ...]\n\
       diction --version\n"),handle);
}
/*}}}*/

int main(int argc, char *argv[]) /*{{{*/
{
  int usage=0,c;
  char *userPhrases=(char*)0,*e;
  char defaultPhrases[_POSIX_PATH_MAX];
  static struct option lopts[]=
  {
    { "ignore-double-words", no_argument, 0, 'd' },
    { "file", required_argument, 0, 'f' },
    { "help", no_argument, 0, 'h' },
    { "version", no_argument, 0, 'v' },
    { "language", required_argument, 0, 'L' },
    { "no-default-file", no_argument, 0, 'n' },
    { (const char*)0, 0, 0, '\0' }
  };

  /* init locale */ /*{{{*/
  setlocale(LC_ALL,"");
#ifdef HAVE_GETTEXT
  bindtextdomain("diction",LOCALEDIR);
  textdomain("diction");
#endif
  e=getenv("LC_MESSAGES");
  if (e==(char*)0) e=getenv("LC_ALL");
  if (e==(char*)0) e=getenv("LANG");
  if (e)
  {
    strncpy(phraseLanguage,e,sizeof(phraseLanguage)-1);
    phraseLanguage[sizeof(phraseLanguage)-1]='\0';
    if (strstr(phraseLanguage,".."))
    {
      fprintf(stderr,_("diction: Invalid string `..' in default phrase language `%s'.\n"),phraseLanguage);
      exit(2);
    }
    else
    {
      snprintf(defaultPhrases,sizeof(defaultPhrases),SHAREDIR "/diction/%s",e);
      if (access(defaultPhrases,R_OK)!=0)
      {
        phraseLanguage[5]='\0';
        snprintf(defaultPhrases,sizeof(defaultPhrases),SHAREDIR "/diction/%s",phraseLanguage);
        if (access(defaultPhrases,R_OK)!=0)
        {
          phraseLanguage[2]='\0';
          snprintf(defaultPhrases,sizeof(defaultPhrases),SHAREDIR "/diction/%s",phraseLanguage);
          if (access(defaultPhrases,R_OK)!=0)
          {
            strcpy(phraseLanguage,"C");
          }
        }
      }
    }
  }
  else strcpy(phraseLanguage,"C");
  /*}}}*/
  /* parse options */ /*{{{*/
  strcpy(defaultPhrases,SHAREDIR "/diction/");
  while ((c=getopt_long(argc,argv,"df:nL:h",lopts,(int*)0))!=EOF) switch(c)
  {
    case 'd': doubleWords=0; break;
    case 'f': userPhrases=optarg; break;
    case 'n': defaultPhrases[0]='\0'; break;
    case 'L': strncpy(phraseLanguage,optarg,sizeof(phraseLanguage)-1); phraseLanguage[sizeof(phraseLanguage)-1]='\0'; break;
    case 'v': printf("GNU diction " VERSION "\n"); exit(0);
    case 'h': usage=2; break;
    default: usage=1; break;
  }
  if (defaultPhrases[0]) strcat(defaultPhrases,phraseLanguage);
  if (usage==1 || (userPhrases==(char*)0 && defaultPhrases[0]=='\0'))
  {
    print_usage(stderr);
    fputs("\n",stderr);
    fputs(_("Try `diction -h' or `diction --help' for more information.\n"),stderr);
    exit(1);
  }
  if (usage==2)
  {
    print_usage(stdout);
    fputs("\n",stdout);
    fputs(_("\
Print wordy and commonly misused phrases in sentences.\n\n\
-d, --ignore-double-words  do not complain about double words\n\
-f, --file                 also read the specified database\n\
-n, --no-default-file      do not read the default phrase file\n\
-L, --language             set document language\n\
-h, --help                 print this message\n\
    --version              print the version\n"),stdout);
    fputs("\n",stdout);
    fputs(_("Report bugs to <michael@moria.de>.\n"),stdout);
    exit(0);
  }
  /*}}}*/
  if (defaultPhrases[0]) loadPhrases(defaultPhrases);
  if (userPhrases) loadPhrases(userPhrases);
  sentences=0;
  hits=0;
  if (optind==argc) sentence("diction",stdin,"(stdin)",diction,phraseLanguage);
  else while (optind<argc)
  {
    FILE *fp;

    if ((fp=fopen(argv[optind],"r"))==(FILE*)0)
    {
      fprintf(stderr,_("diction: Opening `%s' failed (%s).\n"),argv[optind],strerror(errno));
    }
    else
    {
      sentence("diction",fp,argv[optind],diction,phraseLanguage);
      fclose(fp);
    }
    ++optind;
  }
  if (sentences==0)
  {
    printf(_("No sentences found.\n"));
  }
  else
  {
    if (hits==0) printf(_("No phrases "));
    else if (hits==1) printf(_("1 phrase "));
    else printf(_("%d phrases "),hits);
    if (sentences==1) printf(_("in 1 sentence found.\n"));
    else printf(_("in %d sentences found.\n"),sentences);
  }
  exit(0);
}
/*}}}*/
