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

#ifndef SENTENCE_H
#define SENTENCE_H

#include <sys/types.h>
#include <stdio.h>

void sentence(const char *cmd, FILE *in, const char *file, void (*process)(const char *, size_t, const char *, int), const char *lang);

#endif
