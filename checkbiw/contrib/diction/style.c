/* Notes */ /*{{{C}}}*//*{{{*/
/*

This program is GNU software, copyright 1997-2003
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#ifdef HAVE_GETTEXT
#include <libintl.h>
#define _(String) gettext(String)
#else
#define _(String) String
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "misc.h"
#include "sentence.h"
/*}}}*/

/* variables */ /*{{{*/
static const char *lc_ctype;
enum lc_ctype_int { ASCII, ISO_8859_1 };
static enum lc_ctype_int lc_ctype_int;
static const char *docLanguage;
static const char *phraseEnd = (const char*)0;
/*}}}*/

/* hit counting functions */ /*{{{*/
struct Hit /*{{{*/
{
  int *data;
  int capacity;
  int size;
};
/*}}}*/
static void newHit(struct Hit *hit) /*{{{*/
{
  if ((hit->data=malloc((hit->capacity=3)*sizeof(int)))==(int*)0)
  {
    fprintf(stderr,_("style: out of memory\n"));
    exit(1);
  }
  memset(hit->data,0,hit->capacity*sizeof(int));
  hit->size=0;
}
/*}}}*/
static void noteHit(struct Hit *hit, int n) /*{{{*/
{
  assert(n>0);
  if (n>hit->capacity)
  {
    if ((hit->data=realloc(hit->data,n*2*sizeof(int)))==(int*)0)
    {
      fprintf(stderr,_("style: out of memory\n"));
      exit(1);
    }
    memset(hit->data+hit->capacity,0,(n*2-hit->capacity)*sizeof(int));
    hit->capacity=n*2;
  }
  ++hit->data[n-1];
  if (n>hit->size) hit->size=n;
}
/*}}}*/
/*}}}*/
/* readability formulas */ /*{{{*/
/**
 * Calculate Kincaid Formula (reading grade).
 * @param syllables number of syllables
 * @param words number of words
 * @param sentences number of sentences
 */
static double kincaid(int syllables, int words, int sentences) /*{{{*/
{
  return 11.8*(((double)syllables)/words)+0.39*(((double)words)/sentences)-15.59;
}
/*}}}*/

/**
 * Calculate Automated Readability Index (reading grade).
 * @param letters number of letters
 * @param words the number of words
 * @param sentences the number of sentences
 */
static double ari(int letters, int words, int sentences) /*{{{*/
{
  return 4.71*(((double)letters)/words)+0.5*(((double)words)/sentences)-21.43;
}
/*}}}*/

/**
 * Calculate Coleman-Liau Formula.
 * @param letters number of letters
 * @param words the number of words
 * @param sentences the number of sentences
 */
static double coleman_liau(int letters, int words, int sentences) /*{{{*/
{
  return 5.89*(((double)letters)/words)-0.3*(((double)sentences)/(100*words))-15.8;
}
/*}}}*/

/**
 * Calculate Flesch reading ease formula.
 * @param syllables number of syllables
 * @param words number of words
 * @param sentences number of sentences
 */
static double flesch(int syllables, int words, int sentences) /*{{{*/
{
  return 206.835-84.6*(((double)syllables)/words)-1.015*(((double)words)/sentences);
}
/*}}}*/

/**
 * Calculate fog index.
 * @param words the number of words in the text
 * @param bigwords the number of words which contain more than 3 syllables
 * @param sentences the number of sentences
 */
static double fog(int words, int bigwords, int sentences) /*{{{*/
{
  return ((((double)words)/sentences+(100.0*bigwords)/words)*0.4);
}
/*}}}*/

/**
 * Calculate 1. neue Wiener Sachtextformel (WSTF).  MIGHT BE WRONG!
 * @param words the number of words in the text
 * @param shortwords the number of words that contain one syllable
 * @param longwords the number of words that are longer than 6 characters
 * @param bigwords the number of words that contain more than 3 syllables
 * @param sentences number of sentences
 */
static double wstf(int words, int shortwords, int longwords, int bigwords, int sentences) /*{{{*/
{
  return 0.1935*((double)bigwords)/words+0.1672*((double)words)/sentences-0.1297*((double)longwords)/words-0.0327*((double)shortwords)/words-0.875;
}
/*}}}*/

/**
 * Calculate Wheeler-Smith formula.  MIGHT BE WRONG!
 * @param words the number of words in the text
 * @param bigwords the number of words that contain more than 3 syllables
 * @param sentences number of sentences
 * @returns the wheeler smith index as result and the grade level in grade.
 *          If grade is 0, the index is lower than any grade, if the index is
 *          99, it is higher than any grade.
 */
static double wheeler_smith(int *grade, int words, int bigwords, int sentences) /*{{{*/
{
  double idx=(((double)words)/sentences) * 10.0 * (((double)bigwords)/words);

  if (idx<=16) *grade=0;
  else if (idx<=20) *grade=5;
  else if (idx<=24) *grade=6;
  else if (idx<=29) *grade=7;
  else if (idx<=34) *grade=8;
  else if (idx<=38) *grade=9;
  else if (idx<=42) *grade=10;
  else *grade=99;
  return idx;
}
/*}}}*/

/**
 * Calculate Lix formula of Björnsson from Sweden.
 * @param words the number of words in the text
 * @param sentences number of sentences
 * @param longwords the number of words that are longer than 6 characters
 * @returns the wheeler smith index as result and the grade level in grade.
 *          If grade is 0, the index is lower than any grade, if the index is
 *          99, it is higher than any grade.
 */
static double lix(int *grade, int words, int longwords, int sentences) /*{{{*/
{
  double idx=((double)words)/sentences+100.0*((double)longwords)/words;

  if (idx<34) *grade=0;
  else if (idx<38) *grade=5;
  else if (idx<41) *grade=6;
  else if (idx<44) *grade=7;
  else if (idx<48) *grade=8;
  else if (idx<51) *grade=9;
  else if (idx<54) *grade=10;
  else if (idx<57) *grade=11;
  else *grade=99;
  return idx;
}
/*}}}*/

/**
 * Calculate SMOG-Grading.
 * @param bigwords the number of words that contain more than 3 syllables
 * @param sentences number of sentences
 */
static double smog(int bigwords, int sentences) /*{{{*/
{
  if (strncmp(docLanguage,"de",2)==0) return sqrt((((double)bigwords)/((double)sentences))*30)-2.0;
  else return sqrt((((double)bigwords)/((double)sentences))*30.0)+3.0;
}
/*}}}*/
/*}}}*/
/* word class checks */ /*{{{*/
static int wordcmp(const char *r, const char *s) /*{{{*/
{
  int res;

  while (*r)
  {
    if ((res=*r-tolower(*s))!=0) return res;
    ++r; ++s;
  }
  return isalpha(*s);
}
/*}}}*/

/**
 * Test if the word is an article.  This function uses docLanguage to
 * determine the used language.
 */
static int article(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* German articles */ /*{{{*/
  {
    "der", "die", "das", "des", "dem", "den", "ein", "eine", "einer",
    "eines", "einem", "einen", (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* English articles */ /*{{{*/
  {
    "the", "a", "an", (const char*)0
  };
  /*}}}*/
  const char **list;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list) if (wordcmp(*list,word)==0) return 1; else ++list;
  return 0;
}
/*}}}*/

/**
 * Test if the word is a pronoun.  This function uses docLanguage to
 * determine the used language.
 */
static int pronoun(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* Pronomen */ /*{{{*/
  {
    "ich", "du", "er", "sie", "es", "wir", "ihr", "mein", "meine", "dein",
    "deine", "sein", "seine", "unser", "unsere", "euer", "eure", "mir",
    "mich", "dir", "dich", "ihre", (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* pronouns */ /*{{{*/
  {
    "i", "me", "we", "us", "you", "he", "him", "she", "her", "it", "they",
    "them", "thou", "thee", "ye", "myself", "yourself", "himself",
    "herself", "itself", "ourselves", "yourselves", "themselves",
    "oneself", "my", "mine", "his", "hers", "yours", "ours", "theirs", "its",
    "our", "that", "their", "these", "this", "those", "your", (const char*)0
  };
  /*}}}*/
  const char **list;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list) if (wordcmp(*list,word)==0) return 1; else ++list;
  return 0;
}
/*}}}*/

/**
 * Test if the word is an interrogative pronoun.  This function uses
 * docLanguage to determine the used language.
 */
static int interrogativePronoun(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* Interrogativpronomen */ /*{{{*/
  {
    "wer", "was", "wem", "wen", "wessen", "wo", "wie", "warum", "weshalb",
    "wann", "wieso", "weswegen", (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* interrogative pronouns */ /*{{{*/
  {
    "why", "who", "what", "whom", "when", "where", "how", (const char*)0
  };
  /*}}}*/
  const char **list;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list) if (wordcmp(*list,word)==0) return 1; else ++list;
  return 0;
}
/*}}}*/

static int conjunction(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* Konjunktionen */ /*{{{*/
  {
    "und", "oder", "aber", "sondern", "doch", "nur", "bloß", "denn",
    "weder", "noch", "sowie", (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* conjunctions */ /*{{{*/
  {
    "and", "but", "or", "yet", "nor", (const char*)0
  };
  /*}}}*/
  const char **list;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list) if (wordcmp(*list,word)==0) return 1; else ++list;
  return 0;
}
/*}}}*/

static int nominalization(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* Nominalisierungsendungen */ /*{{{*/
  {
    "ung", "heit", "keit", "nis", "tum", (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* nominalization suffixes */ /*{{{*/
  {
     /* a bit limited, but it is exactly what the original style(1) did */
     "tion", "ment", "ence", "ance", (const char*)0
  };
  /*}}}*/
  const char **list;

  /* exclude words too short to have such long suffixes */
  if (l < 7) return 0;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list) if (wordcmp(*list,word+l-strlen(*list))==0) return 1; else ++list;
  return 0;
}
/*}}}*/

static int subConjunction(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* unterordnende Konjunktionen */ /*{{{*/
  {
    "weil", "da", "dadurch", "wenn", "falls", "sofern", "obwohl",
    "obgleich", "als", "nachdem", "während", "wie", "ob", "je",
    "desto", "damit", "dass", "indem", "um zu", (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* subordinating conjunctions */ /*{{{*/
  {
    "after","because", "lest", "till", "'til", "although", "before", 
    "now that", "unless", "as", "even if", "provided that", "provided", 
    "until", "as if", "even though", "since", "as long as", "so that",
    "whenever", "as much as", "if", "than", "as soon as", "inasmuch",
    "in order that", "though", "while", (const char*)0
  };
  /*}}}*/
  const char **list;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list)
  {
    if (wordcmp(*list,word)==0) 
    {
      phraseEnd = word+strlen(*list);
      return 1; 
    }
    else ++list;
  }
  return 0;
}
/*}}}*/

static int preposition(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* Präpositionen */ /*{{{*/
  {
    "aus", "außer", "bei", "mit", "nach", "seit", "von", "zu",
    "bis", "durch", "für", "gegen", "ohne", "um", "an", "auf",
    "hinter", "in", "neben", "über", "unter", "vor", "zwischen",
    "anstatt", "statt", "trotz", "während", "wegen", (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* prepositions */ /*{{{*/
  {
    "aboard", "about", "above", "according to", "across from",
    "after", "against", "alongside", "alongside of", "along with",
    "amid", "among", "apart from", "around", "aside from", "at", "away from",
    "back of", "because of", "before", "behind", "below", "beneath", "beside",
    "besides", "between", "beyond", "but", "by means of",
    "concerning", "considering", "despite", "down", "down from", "during",
    "except", "except for", "excepting for", "from among",
    "from between", "from under", "in addition to", "in behalf of",
    "in front of", "in place of", "in regard to", "inside of", "inside",
    "in spite of", "instead of", "into", "like", "near to", "off",
    "on account of", "on behalf of", "onto", "on top of", "on", "opposite",
    "out of", "out", "outside", "outside of", "over to", "over", "owing to",
    "past", "prior to", "regarding", "round about", "round",
    "since", "subsequent to", "together", "with", "throughout", "through",
    "till", "toward", "under", "underneath", "until", "unto", "up",
    "up to", "upon", "with", "within", "without", "across", "along",
    "by", "of", "in", "to", "near", "of", "from",  (const char*)0
  };
  /*}}}*/
  const char **list;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list)
  {
    if (wordcmp(*list,word)==0)
    {
      phraseEnd = word+strlen(*list);
      return 1; 
    }
    else ++list;
  }
  return 0;
}
/*}}}*/

static int auxVerb(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* Hilfsverben */ /*{{{*/
  {
    "haben", "habe", "hast", "hat", "habt", "gehabt", "hätte", "hättest",
    "hätten", "hättet",
    "werden", "werde", "wirst", "wird", "werdet", "geworden", "würde",
    "würdest", "würden", "würdet",
    "können", "kann", "kannst", "könnt", "konnte", "konntest", "konnten",
    "konntet", "gekonnt", "könnte", "könntest", "könnten", "könntet",
    "müssen", "muss", "musst", "müsst", "musste", "musstest", "mussten",
    "gemusst", "müsste", "müsstest", "müssten", "müsstet",
    "sollen", "soll", "sollst", "sollt", "sollte", "solltest", "solltet",
    "sollten", "gesollt",
    (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* auxiliary verbs */ /*{{{*/
  {
    "will", "shall", "cannot", "may", "need to", "would", "should",
    "could", "might", "must", "ought", "ought to", "can't", "can",
    (const char*)0
  };
  /*}}}*/
  const char **list;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list)
  {
    if (wordcmp(*list,word)==0) 
    {
      phraseEnd = word+strlen(*list);
      return 1; 
    }
    else ++list;
  }
  return 0;
}
/*}}}*/

static int tobeVerb(const char *word, size_t l) /*{{{*/
{
  static const char *de[]= /* Hilfsverb sein */ /*{{{*/
  {
    "sein", "bin", "bist", "ist", "sind", "seid", "war", "warst", "wart",
    "waren", "gewesen", "wäre", "wärst", "wär", "wären", "wärt", "wäret",
    (const char*)0
  };
  /*}}}*/
  static const char *en[]= /* auxiliary verb to be */ /*{{{*/
  {
     "be", "being", "was", "were", "been", "are", "is", (const char*)0
  };
  /*}}}*/
  const char **list;

  if (strncmp(docLanguage,"de",2)==0) list=de;
  else list=en;
  while (*list) if (wordcmp(*list,word)==0) return 1; else ++list;
  return 0;
}
/*}}}*/
/*}}}*/
/* syllable counting */ /*{{{*/
/**
 * Check if the character is pronounced as a vowel.
 */
static int vowel(char c) /*{{{*/
{
  switch (lc_ctype_int)
  {
    case ASCII: return (c=='a' || c=='e' || c=='i' || c=='o' || c=='u' || c=='y');
    case ISO_8859_1: return (c=='a' || c=='ä' || c=='e' || c=='i' || c=='o' || c=='ö' || c=='u' || c=='ü' || c=='y');
    default: assert(0);
  }
}
/*}}}*/

/**
 * Count syllables for english words by counting vowel-consonant pairs.
 * @param s the word
 * @param l the word's length
 */
static int syll_en(const char *s, size_t l) /*{{{*/
{
  int count=0;

  if (l>=2 && *(s+l-2)=='e' && *(s+l-1)=='d') l-=2;
  while (l)
  {
    if (l>=2 && vowel(*s) && !vowel(*(s+1))) { ++count; s+=2; l-=2; }
    else { ++s; --l; }
  }
  return (count==0 ? 1 : count);
}
/*}}}*/

/**
 * Count syllables for German words by counting vowel-consonant or
 * consonant-vowel pairs, depending on the first character being a vowel or
 * not.  If it is, a trailing e will be handled with a special rule.  This
 * algorithm fails on "vor-ueber".
 * @param s the word
 * @param l the word's length
 */
static int syll_de(const char *s, size_t l) /*{{{*/
{  
  int count=0;
  size_t ol=l;
 
  if (vowel(*s))  
  while (l) 
  {
    if (l>=2 && vowel(*s) && !vowel(*(s+1))) { ++count; s+=2; l-=2; }
    else if (l==1 && ol>1 && !vowel(*(s-1)) && *s=='e') { ++count; s+=1; l-=1; }
    else { ++s; --l; }
  }
  else
  while (l)
  {
    if (l>=2 && !vowel(*s) && vowel(*(s+1))) { ++count; s+=2; l-=2; }
    else { ++s; --l; }
  }
  return (count==0 ? 1 : count);
}
/*}}}*/

/**
 * Count syllables.  First, charset is set to the used character set.
 * Depending on the language, the right counting function is called.
 * @param s the word
 * @param l the word's length
 */
static int syll(const char *s, size_t l) /*{{{*/
{
  assert(s!=(const char*)0);
  assert(l>=1);
  if (strncmp(docLanguage,"de",2)==0) return syll_de(s,l);
  else return syll_en(s,l);
}
/*}}}*/
/*}}}*/

/* global style() variables */ /*{{{*/
static int characters;
static int syllables;
static int words;
static int shortwords;
static int longwords;
static int bigwords;
static int sentences;
static int questions;
static int passiveSent;
static int beginArticles;
static int beginPronouns;
static int pronouns;
static int beginInterrogativePronouns;
static int interrogativePronouns;
static int beginConjunctions;
static int conjunctions;
static int nominalizations;
static int prepositions;
static int beginPrepositions;
static int beginSubConjunctions;
static int subConjunctions;
static int auxVerbs;
static int tobeVerbs;
static int shortestLine,shortestLength;
static int longestLine,longestLength;
static int paragraphs;
static int printLongSentences=0;
static int printNomSentences=0;
static int printPassiveSentences=0;
static float printARI=0.0;
static struct Hit lengths;
/*}}}*/

/**
 * Process one sentence.
 * @param str sentence
 * @param length its length
 */
static void style(const char *str, size_t length, const char *file, int line) /*{{{*/
{
  int firstWord=1;
  int inword=0;
  int innumber=0;
  int wordLength=-1;
  int sentWords=0;
  int sentLetters=0;
  int count;
  int passive=0;
  int nom=0;
  const char *s=str;

  if (length==0) { ++paragraphs; return; }
  assert(str!=(const char*)0);
  assert(length>=2);
  phraseEnd = (const char*)0;
  while (*s)
  {
    if (inword)
    {
      if (!isalpha(*s))
      {
        inword=0;
        count=syll(s-wordLength,wordLength);
        syllables+=count;
        if (count>=3) ++bigwords;
        else if (count==1) ++shortwords;
        if (wordLength>6) ++longwords;
        if (s-wordLength > phraseEnd)
        {
          /* part of speech tagging-- order matters! */
          if (article(s-wordLength,wordLength) && firstWord) ++beginArticles;
          else if (pronoun(s-wordLength,wordLength))
          {
            ++pronouns;
            if (firstWord) ++beginPronouns;
          }
          else if (interrogativePronoun(s-wordLength,wordLength))
          { 
            ++interrogativePronouns;
            if (firstWord) ++beginInterrogativePronouns;
          }
          else if (conjunction(s-wordLength,wordLength)) 
          { 
            ++conjunctions;
            if (firstWord) ++beginConjunctions;
          }
          else if (subConjunction(s-wordLength,wordLength)) 
          { 
            ++subConjunctions;
            if (firstWord) ++beginSubConjunctions;
          }
          else if (preposition(s-wordLength,wordLength)) 
          { 
            ++prepositions;
            if (firstWord) ++beginPrepositions;
          }
          else if (tobeVerb(s-wordLength,wordLength))
          { 
            ++passive;
            ++tobeVerbs;
          }
          else if (auxVerb(s-wordLength,wordLength)) ++auxVerbs;
          else if (nominalization(s-wordLength,wordLength))
          { 
            ++nom;
            ++nominalizations;
          }
        }
        if (firstWord) firstWord = 0;
      }
      else
      {
        ++wordLength;
        ++characters;
        ++sentLetters;
      }
    }
    else if (innumber)
    {
      if (!isdigit(*s))
      {
        innumber=0;
        ++syllables;
      }
      else
      {
        ++wordLength;
        ++characters;
        ++sentLetters;
      }
    }
    else
    {
      if (isalpha(*s))
      {
        ++words;
        ++sentWords;
        inword=1;
        wordLength=1;
        ++characters;
        ++sentLetters;
      }
      else if (isdigit(*s))
      {
        ++words;
        ++sentWords;
        innumber=1;
        wordLength=1;
        ++characters;
        ++sentLetters;
      }
    }
    ++s;
  }
  ++sentences;
  if (shortestLine==0 || sentWords<shortestLength)
  {
    shortestLine=sentences;
    shortestLength=sentWords;
  }
  if (longestLine==0 || sentWords>longestLength)
  {
    longestLine=sentences;
    longestLength=sentWords;
  }
  if (str[length-1]=='?') ++questions;
  noteHit(&lengths,sentWords);
  if (passive) ++passiveSent;
  if ((printLongSentences && sentWords>=printLongSentences) 
      || (printARI && ari(sentLetters,sentWords,1)>printARI)
      || (printPassiveSentences && passive)
      || (printNomSentences && nom)) printf("%s:%d: %s\n",file,line,str);
}
/*}}}*/

static void print_usage(FILE *handle) /*{{{*/
{
  fputs(_("\
Usage: style [-L language] [-l length] [-r ari] [file ...]\n\
       style [--language language] [--print-long length] [--print-ari ari]\n\
             [file ...]\n\
       style --version\n"),handle);
}
/*}}}*/

int main(int argc, char *argv[]) /*{{{*/
{
  /* variables */ /*{{{*/
  int usage=0,c;
  static struct option lopts[]=
  {
    { "help", no_argument, 0, 'h' },
    { "print-long", required_argument, 0, 'l' },
    { "language", required_argument, 0, 'L' },
    { "print-ari", required_argument, 0, 'r' },
    { "version", no_argument, 0, 'v' },
    { "print-passive", no_argument, 0, 'p' },
    { "print-nom", no_argument, 0, 'N' },
    { "print-nom-passive", no_argument, 0, 'N' },
    { (const char*)0, 0, 0, '\0' }
  };
  /*}}}*/

  /* locale */ /*{{{*/
  setlocale(LC_ALL,"");
#ifdef HAVE_GETTEXT
  bindtextdomain("diction", LOCALEDIR);
  textdomain("diction");
#endif
  /*}}}*/
  /* parse options */ /*{{{*/
#if 0
  lc_ctype=setlocale(LC_CTYPE,(const char*)0);
  docLanguage=setlocale(LC_MESSAGES,(const char*)0);
#else
  if ((lc_ctype=getenv("LC_CTYPE"))==(const char*)0) lc_ctype="C";
  if ((docLanguage=getenv("LC_MESSAGES"))==(const char*)0) docLanguage="C";
#endif
  if (strcmp(docLanguage,"C")==0) docLanguage="en";
  if (strstr(lc_ctype,"8859-1")) lc_ctype_int=ISO_8859_1;
  else lc_ctype_int=ASCII;
  while ((c=getopt_long(argc,argv,"l:L:r:hpnN",lopts,(int*)0))!=EOF) switch(c)
  {
    case 'l':
    {
      char *end;
      printLongSentences=strtol(optarg,&end,10);
      if (end==optarg || *end!='\0') usage=1;
      break;
    }
    case 'L':
    {
      docLanguage=optarg;
      break;
    }
    case 'r':
    {
      char *end;
      printARI=strtod(optarg,&end);
      if (end==optarg || *end!='\0') usage=1;
      break;
    }
    case 'p':
    { 
      printPassiveSentences=1; 
      break;
    }
    case 'N':
    { 
      printNomSentences=1; 
      break;
    }
    case 'n':
    { 
      printNomSentences=1; 
      printPassiveSentences=1; 
      break;
    }
    case 'v': fputs("GNU style " VERSION "\n",stdout); exit(0);
    case 'h': usage=2; break;
    default: usage=1; break;
  }
  if (usage==1)
  {
    print_usage(stderr);
    fputs("\n",stderr);
    fputs(_("Try style -h|--help for more information.\n"),stderr);
    exit(1);
  }
  else if (usage==2)
  {
    print_usage(stdout);
    fputs("\n",stdout);
    fputs(_("Analyse surface characteristics of a document.\n\n"),stdout);
    fputs(_("\
-L, --language          set the document language.\n\
-l, --print-long        print all sentences longer than <length> words\n\
-r, --print-ari         print all sentences with an ARI greater than than <ari>\n\
-p, --print-passive     print all sentences phrased in the passive voice\n\
-N, --print-nom         print all sentences containing nominalizations\n\
-n, --print-nom-passive print all sentences phrased in the passive voice or\n\
                        containing nominalizations\n"),stdout);
    fputs(_("\
-h, --help              print this message\n\
    --version           print the version\n"),stdout);
    fputs("\n",stdout);
    fputs(_("Report bugs to <michael@moria.de>.\n"),stdout);
    exit(0);
  }
  /*}}}*/
  newHit(&lengths);
  if (optind==argc) sentence("style",stdin,"(stdin)",style,docLanguage);
  else while (optind<argc)
  {
    FILE *fp;
    if ((fp=fopen(argv[optind],"r"))==(FILE*)0) 
      fprintf(stderr,_("style: Opening `%s' failed (%s).\n"),argv[optind],strerror(errno));
    else
    {
      sentence("style",fp,argv[optind],style,docLanguage);
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
    int wsg;
    int lixg;
    int i,shortLength,shortSent,longLength,longSent;

    printf(_("readability grades:\n"));
    printf("        %s: %.1f\n",_("Kincaid"),kincaid(syllables,words,sentences));
    printf("        %s: %.1f\n",_("ARI"),ari(characters,words,sentences));
    printf("        %s: %.1f\n",_("Coleman-Liau"),coleman_liau(characters,words,sentences));
    printf("        %s: %.1f\n",_("Flesch Index"),flesch(syllables,words,sentences));
    printf("        %s: %.1f\n",_("Fog Index"),fog(words,bigwords,sentences));
#ifdef MIGHT_BE_WRONG
    printf("        %s: %.1f\n",_("1. WSTF Index"),wstf(words,shortwords,longwords,bigwords,sentences));
    printf("        %s: %.1f = ",_("Wheeler-Smith Index"),wheeler_smith(&wsg,words,bigwords,sentences));
    if (wsg==0) printf(_("below school year 5\n"));
    else if (wsg==99) printf(_("higher than school year 10\n"));
    else printf(_("school year %d\n"),wsg);
#endif
    printf("        %s: %.1f = ",_("Lix"),lix(&lixg,words,longwords,sentences));
    if (lixg==0) printf(_("below school year 5\n"));
    else if (lixg==99) printf(_("higher than school year 11\n"));
    else printf(_("school year %d\n"),lixg);
    printf("        %s: %.1f\n",_("SMOG-Grading"),smog(bigwords,sentences));

    printf(_("sentence info:\n"));
    printf(_("        %d characters\n"),characters);
    printf(_("        %d words, average length %.2f characters = %.2f syllables\n"),words,((double)characters)/words,((double)syllables)/words);
    printf(_("        %d sentences, average length %.1f words\n"),sentences,((double)words)/sentences);
    shortLength=((double)words)/sentences-4.5;
    if (shortLength<1) shortLength=1;
    for (i=0,shortSent=0; i<=shortLength; ++i) shortSent+=lengths.data[i];
    printf(_("        %d%% (%d) short sentences (at most %d words)\n"),100*shortSent/sentences,shortSent,shortLength);
    longLength=((double)words)/sentences+10.5;
    for (i=longLength,longSent=0; i<=lengths.size; ++i) longSent+=lengths.data[i];
    printf(_("        %d%% (%d) long sentences (at least %d words)\n"),100*longSent/sentences,longSent,longLength);
    printf(_("        %d paragraphs, average length %.1f sentences\n"),paragraphs,((double)sentences)/paragraphs);
    printf(_("        %d%% (%d) questions\n"),100*questions/sentences,questions);
    printf(_("        %d%% (%d) passive sentences\n"),100*passiveSent/sentences,passiveSent);
    printf(_("        longest sent %d wds at sent %d; shortest sent %d wds at sent %d\n"),longestLength,longestLine,shortestLength,shortestLine);

/*
Missing output:

sentence types:
        simple 100% (1) complex   0% (0)
        compound   0% (0) compound-complex   0% (0)
word usage:
        verb types as % of total verbs
        tobe 100% (1) aux   0% (0) inf   0% (0)
        passives as % of non-inf verbs   0% (0)
        types as % of total
        prep 0.0% (0) conj 0.0% (0) adv 0.0% (0)
        noun 25.0% (1) adj 25.0% (1) pron 25.0% (1)
        nominalizations   0 % (0)
*/
        if (strncmp(docLanguage,"en",2)==0)
        {
          printf(_("word usage:\n"));
          printf(_("        verb types:\n"));
          printf(_("        to be (%d) auxiliary (%d) \n"), tobeVerbs, auxVerbs);
          printf(_("        types as %% of total:\n"));
          printf(_("        conjunctions %1.f% (%d) pronouns %1.f% (%d) prepositions %1.f% (%d)\n"), 
                  (100.0*(conjunctions+subConjunctions))/words, 
                  conjunctions+subConjunctions, 
                  (100.0*pronouns)/words, pronouns, (100.0*prepositions)/words,
                  prepositions);
          printf(_("        nominalizations %1.f% (%d)\n"),
                  (100.0*nominalizations)/words, nominalizations);
        }

        printf(_("sentence beginnings:\n"));
        printf(_("        pronoun (%d) interrogative pronoun (%d) article (%d)\n"),beginPronouns,beginInterrogativePronouns,beginArticles);
        if (strncmp(docLanguage,"en",2)==0)
        {
          printf(_("        subordinating conjunction (%d) conjunction (%d) preposition (%d)\n"), beginSubConjunctions,beginConjunctions,beginPrepositions);
        }

/*
        subject opener: noun (0) pron (1) pos (0) adj (0) art (0) tot 100%
        prep   0% (0) adv   0% (0)
        verb   0% (0)  sub_conj   0% (0) conj   0% (0)
        expletives   0% (0)
*/
  }
  exit(0);
}
/*}}}*/
