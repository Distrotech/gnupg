/* mkdtemp.c - libc replacement function
 * Copyright (C) 2001 Free Software Foundation, Inc.
 *
 * This file is part of GnuPG.
 *
 * GnuPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GnuPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* This is a replacement function for mkdtemp in case the platform
   we're building on (like mine!) doesn't have it. */

#include <config.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <gcrypt.h>

#ifdef MKDIR_TAKES_ONE_ARG
# undef mkdir
# define mkdir(a,b) mkdir(a)
#endif

char *mkdtemp(char *template)
{
  int attempts,idx,count=0;
  unsigned char *ch;

  idx=strlen(template);

  /* Walk backwards to count all the Xes */
  while(idx>0 && template[idx-1]=='X')
    {
      count++;
      idx--;
    }

  if(count==0)
    {
      errno=EINVAL;
      return NULL;
    }

  ch=&template[idx];

  /* Try 4 times to make the temp directory */
  for(attempts=0;attempts<4;attempts++)
    {
      int remaining=count;
      char *marker=ch;
      unsigned char *randombits;

      idx=0;

      /* Using really random bits is probably overkill here.  The
	 worst thing that can happen with a directory name collision
	 is that the function will return an error. */

      randombits = gcry_random_bytes (4*remaining, GCRY_WEAK_RANDOM);

      while(remaining>1)
	{
	  sprintf(marker,"%02X",randombits[idx++]);
	  marker+=2;
	  remaining-=2;
	}

      /* Any leftover Xes?  get_random_bits rounds up to full bytes,
         so this is safe. */
      if(remaining>0)
	sprintf(marker,"%X",randombits[idx]&0xF);

      gcry_free (randombits);

      if(mkdir(template,0700)==0)
	break;
    }

  if(attempts==4)
    return NULL; /* keeps the errno from mkdir, whatever it is */

  return template;
}


