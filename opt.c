/* Getopt for GNU.
   NOTE: getopt is now part of the C library, so if you don't know what
   "Keep this file name-space clean" means, talk to roland@gnu.ai.mit.edu
   before changing it!

   Copyright (C) 1987, 88, 89, 90, 91, 92, 93, 94
   	Free Software Foundation, Inc.

This file is part of the GNU C Library.  Its master source is NOT part of
the C library, however.  The master source lives in /gd/gnu/lib.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* This tells Alpha OSF/1 not to define a getopt prototype in <stdio.h>.
   Ditto for AIX 3.2 and <stdlib.h>.  */
#ifndef _NO_PROTO
#define _NO_PROTO
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if !defined (__STDC__) || !__STDC__
/* This is a separate conditional since some stdc systems
   reject `defined (const)'.  */
#ifndef const
#define const
#endif
#endif

#include <stdio.h>
#include <pp-printf.h>
#include <kernel2lm32_layer.h>


/* Comment out all this code if we are using the GNU C Library, and are not
   actually compiling the library itself.  This code is part of the GNU C
   Library, but also included in many other GNU distributions.  Compiling
   and linking in this code is a waste when using the GNU C library
   (especially if it is a shared library).  Rather than having every GNU
   program understand `configure --with-gnu-libc' and omit the object files,
   it is simpler to just do this in the source for each such file.  */

#include <stdlib.h>
#include "opt.h"

char *optarg = NULL;

int optind = 0;

static char *nextchar;

int opterr = 1;

int optopt = '?';


static int first_nonopt;
static int last_nonopt;


char *getenv ();

static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;

/* Value of POSIXLY_CORRECT environment variable.  */
static char *posixly_correct;


#include <string.h>
#define	my_index	strchr

static void exchange (argv)
     char **argv;
{
	int bottom = first_nonopt;
	int middle = last_nonopt;
	int top = optind;
	char *tem;

  /* Exchange the shorter segment with the far end of the longer segment.
     That puts the shorter segment into the right place.
     It leaves the longer segment in the right place overall,
     but it consists of two parts that need to be swapped next.  */

	while (top > middle && middle > bottom)
		{
		if (top - middle > middle - bottom)
			{
				/* Bottom segment is the short one.  */
				int len = middle - bottom;
				register int i;

				/* Swap it with the top part of the top segment.  */
		for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[top - (middle - bottom) + i];
				argv[top - (middle - bottom) + i] = tem;
			}
			/* Exclude the moved bottom segment from further swapping.  */
			top -= len;
		}
		else
		{
			/* Top segment is the short one.  */
			int len = top - middle;
			register int i;

			/* Swap it with the bottom part of the bottom segment.  */
			for (i = 0; i < len; i++)
				{
					tem = argv[bottom + i];
					argv[bottom + i] = argv[middle + i];
					argv[middle + i] = tem;
				}
	  /* Exclude the moved top segment from further swapping.  */
			bottom += len;
		}
    }

  /* Update records for the slots the non-options now occupy.  */

  first_nonopt += (optind - last_nonopt);
  last_nonopt = optind;
}

static const char * _getopt_initialize (optstring)
     const char *optstring;
{

	first_nonopt = last_nonopt = optind = 0;

	nextchar = NULL;

	posixly_correct = getenv ("POSIXLY_CORRECT");

	/* Determine how to handle the ordering of options and nonoptions.  */

	if (optstring[0] == '-'){
		ordering = RETURN_IN_ORDER;
		++optstring;
	}
	else if (optstring[0] == '+'){
		ordering = REQUIRE_ORDER;
		++optstring;
    }
    else if (posixly_correct != NULL)
		ordering = REQUIRE_ORDER;
	else
		ordering = PERMUTE;

  return optstring;
}


int getopt (argc, argv, optstring)
     int argc;
     char *const *argv;
     const char *optstring;
{
	int i;
	optarg = NULL;
		
	/*for (i=0; i<argc; i++)
		pp_printf("Begin: argv[%i]=%s\n", argc, argv[i]);*/
	

	if (optind == 0)
		optstring = _getopt_initialize (optstring);

	if (nextchar == NULL || *nextchar == '\0'){
      /* Advance to the next ARGV-element.  */

		if (ordering == PERMUTE){
			/* If we have just processed some options following some non-options,
			exchange them so that the options come first.  */

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **) argv);
			else if (last_nonopt != optind)
				first_nonopt = optind;

			/* Skip any additional non-options
			and extend the range of non-options previously skipped.  */

			while (optind < argc && (argv[optind][0] != '-' || argv[optind][1] == '\0'))
				optind++;
			last_nonopt = optind;
		} //if (ordering == PERMUTE)
		
		/* 	The special ARGV-element `--' means premature end of options.
			Skip it like a null option,
			then exchange with previous non-options as if it were an option,
			then skip everything else like a non-option.  */
		
		if (optind != argc && !strcmp (argv[optind], "--"))
		{
			optind++;

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **) argv);
			else if (first_nonopt == last_nonopt)
				first_nonopt = optind;
			last_nonopt = argc;

			optind = argc;
		}//if (optind != argc && !strcmp (argv[optind], "--"))
		
		/* If we have done all the ARGV-elements, stop the scan
		and back over any non-options that we skipped and permuted.  */

		if (optind == argc)
		{
		/* Set the next-arg-index to point at the non-options
			that we previously skipped, so the caller will digest them.  */
			//pp_printf("if (optind == argc) => %i == %i\n", optind, argc);
			if (first_nonopt != last_nonopt)
				optind = first_nonopt;
			return EOF;
		}// if (optind == argc)

		/* If we have come to a non-option and did not permute it,
		either stop the scan or describe it to the caller and pass it by.  */

		if ((argv[optind][0] != '-' || argv[optind][1] == '\0'))
		{
			//pp_printf("if ((argv[optind][0] != '-' || argv[optind][1] == '\\0'))\n");
			if (ordering == REQUIRE_ORDER)
				return EOF;
			optarg = argv[optind++];
			return -1;
		}//	if ((argv[optind][0] != '-' || argv[optind][1] == '\0'))

		/* We have found another option-ARGV-element.
		Skip the initial punctuation.  */
		
		
		nextchar = (argv[optind] + 1);
	}//if (nextchar == NULL || *nextchar == '\0')
	
	/* Decode the current option-ARGV-element.  */

	/* Check whether the ARGV-element is a long option.

     If long_only and the ARGV-element has the form "-f", where f is
     a valid short option, don't consider it an abbreviated form of
     a long option that starts with f.  Otherwise there would be no
     way to give the -f short option.

     On the other hand, if there's a long option "fubar" and
     the ARGV-element is "-fu", do consider that an abbreviation of
     the long option, just like "--fu", and not "-f" with arg "u".

     This distinction seems to be the most useful approach.  */

	if ( !my_index (optstring, argv[optind][1]))
    {
		char *nameend;
		const struct option *p;
		const struct option *pfound = NULL;
		int exact = 0;
		int ambig = 0;
		int indfound;
		int option_index;
      
		//pp_printf("if ( !my_index (optstring, argv[optind][1]))\n");
		for (nameend = nextchar; *nameend && *nameend != '='; nameend++);
		
		/* Can't find it as a long option.  If this is not getopt_long_only,
		or the option starts with '--' or is not a valid short
		option, then it's an error.
		Otherwise interpret it as a short option.  */
		if (1 || argv[optind][1] == '-' || my_index (optstring, *nextchar) == NULL)
		{
			if (opterr)
			{
				if (argv[optind][1] == '-')
				/* --option */
					fprintf (stderr, "%s: unrecognized option `--%s'\n", argv[0], nextchar);
				else
				/* +option or -option */
					fprintf (stderr, "%s: unrecognized option `%c%s'\n", argv[0], argv[optind][0], nextchar);
			}
			nextchar = (char *) "";
			optind++;
			return '?';
		} //if (1 || argv[optind][1] == '-' || my_index (optstring, *nextchar) == NULL)	
	} //if ( !my_index (optstring, argv[optind][1]))
	
	{
		char c = *nextchar++;
		char *temp = my_index (optstring, c);

		/* Increment `optind' when we start to process its last character.  */
		if (*nextchar == '\0')
			++optind;

		if (temp == NULL || c == ':')
		{
			if (opterr)
			{
				if (posixly_correct)
				/* 1003.2 specifies the format of this message.  */
					fprintf (stderr, "%s: illegal option -- %c\n", argv[0], c);
				else
					fprintf (stderr, "%s: invalid option -- %c\n", argv[0], c);
			}
			optopt = c;
			return '?';
		}
		if (temp[1] == ':')
		{
			if (temp[2] == ':')
			{
				/* This is an option that accepts an argument optionally.  */
				if (*nextchar != '\0')
				{
					optarg = nextchar;
					optind++;
				}
				else
					optarg = NULL;
				nextchar = NULL;
			}
			else
			{
				/* This is an option that requires an argument.  */
				if (*nextchar != '\0')
				{
					optarg = nextchar;
					/* If we end this ARGV-element by taking the rest as an arg,
					we must advance to the next element now.  */
					optind++;
				}
				else if (optind == argc)
				{
					if (opterr)
					{
						/* 1003.2 specifies the format of this message.  */
						fprintf (stderr, "%s: option requires an argument -- %c\n",
						argv[0], c);
					}
					optopt = c;
					if (optstring[0] == ':')
						c = ':';
					else
						c = '?';
				}
				else
			/* We already incremented `optind' once;
			increment it again when taking next ARGV-elt as argument.  */
				optarg = argv[optind++];
				nextchar = NULL;
			}
		}
		//pp_printf("Reached the end\n");
		return c;
	}
	
}

