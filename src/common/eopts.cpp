#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <eopts.h>

#define BAD_OPTION '\0'
#define	my_index	strchr
#define	my_strlen	strlen

getoptex::getoptex() :
	optarg(NULL),
	optind(0),
	opterr(1),
	optopt(BAD_OPTION),
	nextchar(NULL),
	first_nonopt(0),
	last_nonopt(0),
	ordering(REQUIRE_ORDER)
{
}

getoptex::~getoptex()
{
}

int getoptex::getopt(int argc, const char ** argv, const char * optstring)
{
  return _getopt_internal(argc, argv, optstring, (const struct option *) 0, (int *) 0, 0);
}

int getoptex::getopt_long(int argc, const char ** argv, const char *options, const struct option * long_options, int * opt_index)
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 0);
}

int getoptex::getopt_long_only(int argc, const char ** argv, const char *options, const struct option * long_options, int * opt_index)
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 1);
}

int getoptex::_getopt_internal(
     int argc,
     const char ** argv,
     const char *optstring,
     const struct option *longopts,
     int *longind,
     int long_only)
{
  int option_index;

  optarg = 0;

  if (optind == 0)
    {
      first_nonopt = last_nonopt = optind = 1;

      nextchar = NULL;

      /* Determine how to handle the ordering of options and nonoptions.  */

      if (optstring[0] == '-')
	{
	  ordering = RETURN_IN_ORDER;
	  ++optstring;
	}
      else if (optstring[0] == '+')
	{
	  ordering = REQUIRE_ORDER;
	  ++optstring;
	}
      else if (getenv ("POSIXLY_CORRECT") != NULL)
	ordering = REQUIRE_ORDER;
      else
	ordering = PERMUTE;
    }

  if (nextchar == NULL || *nextchar == '\0')
    {
      if (ordering == PERMUTE)
	{
	  /* If we have just processed some options following some non-options,
	     exchange them so that the options come first.  */

	  if (first_nonopt != last_nonopt && last_nonopt != optind)
	    exchange (argv);
	  else if (last_nonopt != optind)
	    first_nonopt = optind;

	  /* Now skip any additional non-options
	     and extend the range of non-options previously skipped.  */

	  while (optind < argc
		 && (argv[optind][0] != '-' || argv[optind][1] == '\0')
#ifdef GETOPT_COMPAT
		 && (longopts == NULL
		     || argv[optind][0] != '+' || argv[optind][1] == '\0')
#endif				/* GETOPT_COMPAT */
		 )
	    optind++;
	  last_nonopt = optind;
	}

      /* Special ARGV-element `--' means premature end of options.
	 Skip it like a null option,
	 then exchange with previous non-options as if it were an option,
	 then skip everything else like a non-option.  */

      if (optind != argc && !strcmp (argv[optind], "--"))
	{
	  optind++;

	  if (first_nonopt != last_nonopt && last_nonopt != optind)
	    exchange ((const char **) argv);
	  else if (first_nonopt == last_nonopt)
	    first_nonopt = optind;
	  last_nonopt = argc;

	  optind = argc;
	}

      /* If we have done all the ARGV-elements, stop the scan
	 and back over any non-options that we skipped and permuted.  */

      if (optind == argc)
	{
	  /* Set the next-arg-index to point at the non-options
	     that we previously skipped, so the caller will digest them.  */
	  if (first_nonopt != last_nonopt)
	    optind = first_nonopt;
	  return EOF;
	}

      /* If we have come to a non-option and did not permute it,
	 either stop the scan or describe it to the caller and pass it by.  */

      if ((argv[optind][0] != '-' || argv[optind][1] == '\0')
#ifdef GETOPT_COMPAT
	  && (longopts == NULL
	      || argv[optind][0] != '+' || argv[optind][1] == '\0')
#endif				/* GETOPT_COMPAT */
	  )
	{
	  if (ordering == REQUIRE_ORDER)
	    return EOF;
	  optarg = argv[optind++];
	  return 1;
	}

      /* We have found another option-ARGV-element.
	 Start decoding its characters.  */

      nextchar = (argv[optind] + 1
		  + (longopts != NULL && argv[optind][1] == '-'));
    }

  if (longopts != NULL
      && ((argv[optind][0] == '-'
	   && (argv[optind][1] == '-' || long_only))
#ifdef GETOPT_COMPAT
	  || argv[optind][0] == '+'
#endif				/* GETOPT_COMPAT */
	  ))
    {
      const struct option *p;
      const char *s = nextchar;
      int exact = 0;
      int ambig = 0;
      const struct option *pfound = NULL;
      int indfound = 0;

      while (*s && *s != '=')
	s++;

      /* Test all options for either exact match or abbreviated matches.  */
      for (p = longopts, option_index = 0; p->name;
	   p++, option_index++)
	if (!strncmp (p->name, nextchar, s - nextchar))
	  {
	    if (s - nextchar == my_strlen (p->name))
	      {
		/* Exact match found.  */
		pfound = p;
		indfound = option_index;
		exact = 1;
		break;
	      }
	    else if (pfound == NULL)
	      {
		/* First nonexact match found.  */
		pfound = p;
		indfound = option_index;
	      }
	    else
	      /* Second nonexact match found.  */
	      ambig = 1;
	  }

      if (ambig && !exact)
	{
	  if (opterr)
	    fprintf (stderr, "%s: option `%s' is ambiguous\n",
		     argv[0], argv[optind]);
	  nextchar += my_strlen (nextchar);
	  optind++;
	  return BAD_OPTION;
	}

      if (pfound != NULL)
	{
	  option_index = indfound;
	  optind++;
	  if (*s)
	    {
	      /* Don't test has_arg with >, because some C compilers don't
		 allow it to be used on enums.  */
	      if (pfound->has_arg)
		optarg = s + 1;
	      else
		{
		  if (opterr)
		    {
		      if (argv[optind - 1][1] == '-')
			/* --option */
			fprintf (stderr,
				 "%s: option `--%s' doesn't allow an argument\n",
				 argv[0], pfound->name);
		      else
			/* +option or -option */
			fprintf (stderr,
			     "%s: option `%c%s' doesn't allow an argument\n",
			     argv[0], argv[optind - 1][0], pfound->name);
		    }
		  nextchar += my_strlen (nextchar);
		  return BAD_OPTION;
		}
	    }
	  else if (pfound->has_arg == 1)
	    {
	      if (optind < argc)
		optarg = argv[optind++];
	      else
		{
		  if (opterr)
		    fprintf (stderr, "%s: option `%s' requires an argument\n",
			     argv[0], argv[optind - 1]);
		  nextchar += my_strlen (nextchar);
		  return optstring[0] == ':' ? ':' : BAD_OPTION;
		}
	    }
	  nextchar += my_strlen (nextchar);
	  if (longind != NULL)
	    *longind = option_index;
	  if (pfound->flag)
	    {
	      *(pfound->flag) = pfound->val;
	      return 0;
	    }
	  return pfound->val;
	}
      /* Can't find it as a long option.  If this is not getopt_long_only,
	 or the option starts with '--' or is not a valid short
	 option, then it's an error.
	 Otherwise interpret it as a short option.  */
      if (!long_only || argv[optind][1] == '-'
#ifdef GETOPT_COMPAT
	  || argv[optind][0] == '+'
#endif				/* GETOPT_COMPAT */
	  || my_index (optstring, *nextchar) == NULL)
	{
	  if (opterr)
	    {
	      if (argv[optind][1] == '-')
		/* --option */
		fprintf (stderr, "%s: unrecognized option `--%s'\n",
			 argv[0], nextchar);
	      else
		/* +option or -option */
		fprintf (stderr, "%s: unrecognized option `%c%s'\n",
			 argv[0], argv[optind][0], nextchar);
	    }
	  nextchar = (char *) "";
	  optind++;
	  return BAD_OPTION;
	}
    }

  /* Look at and handle the next option-character.  */

  {
    char c = *nextchar++;
    const char *temp = my_index(optstring, c);

    /* Increment `optind' when we start to process its last character.  */
    if (*nextchar == '\0')
      ++optind;

    if (temp == NULL || c == ':')
      {
	if (opterr)
	  {
#if 0
	    if (c < 040 || c >= 0177)
	      fprintf (stderr, "%s: unrecognized option, character code 0%o\n",
		       argv[0], c);
	    else
	      fprintf (stderr, "%s: unrecognized option `-%c'\n", argv[0], c);
#else
	    /* 1003.2 specifies the format of this message.  */
	    fprintf (stderr, "%s: illegal option -- %c\n", argv[0], c);
#endif
	  }
	optopt = c;
	return BAD_OPTION;
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
	      optarg = 0;
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
#if 0
		    fprintf (stderr, "%s: option `-%c' requires an argument\n",
			     argv[0], c);
#else
		    /* 1003.2 specifies the format of this message.  */
		    fprintf (stderr, "%s: option requires an argument -- %c\n",
			     argv[0], c);
#endif
		  }
		optopt = c;
		if (optstring[0] == ':')
		  c = ':';
		else
		  c = BAD_OPTION;
	      }
	    else
	      /* We already incremented `optind' once;
		 increment it again when taking next ARGV-elt as argument.  */
	      optarg = argv[optind++];
	    nextchar = NULL;
	  }
      }
    return c;
  }
}

void getoptex::exchange (const char **argv)
{
  const char *temp, **first, **last;

  /* Reverse all the elements [first_nonopt, optind) */
  first = &argv[first_nonopt];
  last  = &argv[optind-1];
  while (first < last) {
    temp = *first; *first = *last; *last = temp; first++; last--;
  }
  /* Put back the options in order */
  first = &argv[first_nonopt];
  first_nonopt += (optind - last_nonopt);
  last  = &argv[first_nonopt - 1];
  while (first < last) {
    temp = *first; *first = *last; *last = temp; first++; last--;
  }

  /* Put back the non options in order */
  first = &argv[first_nonopt];
  last_nonopt = optind;
  last  = &argv[last_nonopt-1];
  while (first < last) {
    temp = *first; *first = *last; *last = temp; first++; last--;
  }
}


#ifdef WINDOWS

char *optarg = 0;
int optind = 0;
int opterr = 1;
int optopt = BAD_OPTION;

static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;

static char *nextchar;
static int first_nonopt;
static int last_nonopt;


static void exchange (char **argv)
{
  char *temp, **first, **last;

  /* Reverse all the elements [first_nonopt, optind) */
  first = &argv[first_nonopt];
  last  = &argv[optind-1];
  while (first < last) {
    temp = *first; *first = *last; *last = temp; first++; last--;
  }
  /* Put back the options in order */
  first = &argv[first_nonopt];
  first_nonopt += (optind - last_nonopt);
  last  = &argv[first_nonopt - 1];
  while (first < last) {
    temp = *first; *first = *last; *last = temp; first++; last--;
  }

  /* Put back the non options in order */
  first = &argv[first_nonopt];
  last_nonopt = optind;
  last  = &argv[last_nonopt-1];
  while (first < last) {
    temp = *first; *first = *last; *last = temp; first++; last--;
  }
}

/* Scan elements of ARGV (whose length is ARGC) for option characters
   given in OPTSTRING.

   If an element of ARGV starts with '-', and is not exactly "-" or "--",
   then it is an option element.  The characters of this element
   (aside from the initial '-') are option characters.  If `getopt'
   is called repeatedly, it returns successively each of the option characters
   from each of the option elements.

   If `getopt' finds another option character, it returns that character,
   updating `optind' and `nextchar' so that the next call to `getopt' can
   resume the scan with the following option character or ARGV-element.

   If there are no more option characters, `getopt' returns `EOF'.
   Then `optind' is the index in ARGV of the first ARGV-element
   that is not an option.  (The ARGV-elements have been permuted
   so that those that are not options now come last.)

   OPTSTRING is a string containing the legitimate option characters.
   If an option character is seen that is not listed in OPTSTRING,
   return BAD_OPTION after printing an error message.  If you set `opterr' to
   zero, the error message is suppressed but we still return BAD_OPTION.

   If a char in OPTSTRING is followed by a colon, that means it wants an arg,
   so the following text in the same ARGV-element, or the text of the following
   ARGV-element, is returned in `optarg'.  Two colons mean an option that
   wants an optional arg; if there is text in the current ARGV-element,
   it is returned in `optarg', otherwise `optarg' is set to zero.

   If OPTSTRING starts with `-' or `+', it requests different methods of
   handling the non-option ARGV-elements.
   See the comments about RETURN_IN_ORDER and REQUIRE_ORDER, above.

   Long-named options begin with `--' instead of `-'.
   Their names may be abbreviated as long as the abbreviation is unique
   or is an exact match for some defined option.  If they have an
   argument, it follows the option name in the same ARGV-element, separated
   from the option name by a `=', or else the in next ARGV-element.
   When `getopt' finds a long-named option, it returns 0 if that option's
   `flag' field is nonzero, the value of the option's `val' field
   if the `flag' field is zero.

   The elements of ARGV aren't really const, because we permute them.
   But we pretend they're const in the prototype to be compatible
   with other systems.

   LONGOPTS is a vector of `struct option' terminated by an
   element containing a name which is zero.

   LONGIND returns the index in LONGOPT of the long-named option found.
   It is only valid when a long-named option has been found by the most
   recent call.

   If LONG_ONLY is nonzero, '-' as well as '--' can introduce
   long-named options.  */

int _getopt_internal(
     int argc,
     char *const *argv,
     const char *optstring,
     const struct option *longopts,
     int *longind,
     int long_only)
{
  int option_index;

  optarg = 0;

  /* Initialize the internal data when the first call is made.
     Start processing options with ARGV-element 1 (since ARGV-element 0
     is the program name); the sequence of previously skipped
     non-option ARGV-elements is empty.  */

  if (optind == 0)
    {
      first_nonopt = last_nonopt = optind = 1;

      nextchar = NULL;

      /* Determine how to handle the ordering of options and nonoptions.  */

      if (optstring[0] == '-')
	{
	  ordering = RETURN_IN_ORDER;
	  ++optstring;
	}
      else if (optstring[0] == '+')
	{
	  ordering = REQUIRE_ORDER;
	  ++optstring;
	}
      else if (getenv ("POSIXLY_CORRECT") != NULL)
	ordering = REQUIRE_ORDER;
      else
	ordering = PERMUTE;
    }

  if (nextchar == NULL || *nextchar == '\0')
    {
      if (ordering == PERMUTE)
	{
	  /* If we have just processed some options following some non-options,
	     exchange them so that the options come first.  */

	  if (first_nonopt != last_nonopt && last_nonopt != optind)
	    exchange ((char **) argv);
	  else if (last_nonopt != optind)
	    first_nonopt = optind;

	  /* Now skip any additional non-options
	     and extend the range of non-options previously skipped.  */

	  while (optind < argc
		 && (argv[optind][0] != '-' || argv[optind][1] == '\0')
#ifdef GETOPT_COMPAT
		 && (longopts == NULL
		     || argv[optind][0] != '+' || argv[optind][1] == '\0')
#endif				/* GETOPT_COMPAT */
		 )
	    optind++;
	  last_nonopt = optind;
	}

      /* Special ARGV-element `--' means premature end of options.
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
	}

      /* If we have done all the ARGV-elements, stop the scan
	 and back over any non-options that we skipped and permuted.  */

      if (optind == argc)
	{
	  /* Set the next-arg-index to point at the non-options
	     that we previously skipped, so the caller will digest them.  */
	  if (first_nonopt != last_nonopt)
	    optind = first_nonopt;
	  return EOF;
	}

      /* If we have come to a non-option and did not permute it,
	 either stop the scan or describe it to the caller and pass it by.  */

      if ((argv[optind][0] != '-' || argv[optind][1] == '\0')
#ifdef GETOPT_COMPAT
	  && (longopts == NULL
	      || argv[optind][0] != '+' || argv[optind][1] == '\0')
#endif				/* GETOPT_COMPAT */
	  )
	{
	  if (ordering == REQUIRE_ORDER)
	    return EOF;
	  optarg = argv[optind++];
	  return 1;
	}

      /* We have found another option-ARGV-element.
	 Start decoding its characters.  */

      nextchar = (argv[optind] + 1
		  + (longopts != NULL && argv[optind][1] == '-'));
    }

  if (longopts != NULL
      && ((argv[optind][0] == '-'
	   && (argv[optind][1] == '-' || long_only))
#ifdef GETOPT_COMPAT
	  || argv[optind][0] == '+'
#endif				/* GETOPT_COMPAT */
	  ))
    {
      const struct option *p;
      char *s = nextchar;
      int exact = 0;
      int ambig = 0;
      const struct option *pfound = NULL;
      int indfound = 0;

      while (*s && *s != '=')
	s++;

      /* Test all options for either exact match or abbreviated matches.  */
      for (p = longopts, option_index = 0; p->name;
	   p++, option_index++)
	if (!strncmp (p->name, nextchar, s - nextchar))
	  {
	    if (s - nextchar == my_strlen (p->name))
	      {
		/* Exact match found.  */
		pfound = p;
		indfound = option_index;
		exact = 1;
		break;
	      }
	    else if (pfound == NULL)
	      {
		/* First nonexact match found.  */
		pfound = p;
		indfound = option_index;
	      }
	    else
	      /* Second nonexact match found.  */
	      ambig = 1;
	  }

      if (ambig && !exact)
	{
	  if (opterr)
	    fprintf (stderr, "%s: option `%s' is ambiguous\n",
		     argv[0], argv[optind]);
	  nextchar += my_strlen (nextchar);
	  optind++;
	  return BAD_OPTION;
	}

      if (pfound != NULL)
	{
	  option_index = indfound;
	  optind++;
	  if (*s)
	    {
	      /* Don't test has_arg with >, because some C compilers don't
		 allow it to be used on enums.  */
	      if (pfound->has_arg)
		optarg = s + 1;
	      else
		{
		  if (opterr)
		    {
		      if (argv[optind - 1][1] == '-')
			/* --option */
			fprintf (stderr,
				 "%s: option `--%s' doesn't allow an argument\n",
				 argv[0], pfound->name);
		      else
			/* +option or -option */
			fprintf (stderr,
			     "%s: option `%c%s' doesn't allow an argument\n",
			     argv[0], argv[optind - 1][0], pfound->name);
		    }
		  nextchar += my_strlen (nextchar);
		  return BAD_OPTION;
		}
	    }
	  else if (pfound->has_arg == 1)
	    {
	      if (optind < argc)
		optarg = argv[optind++];
	      else
		{
		  if (opterr)
		    fprintf (stderr, "%s: option `%s' requires an argument\n",
			     argv[0], argv[optind - 1]);
		  nextchar += my_strlen (nextchar);
		  return optstring[0] == ':' ? ':' : BAD_OPTION;
		}
	    }
	  nextchar += my_strlen (nextchar);
	  if (longind != NULL)
	    *longind = option_index;
	  if (pfound->flag)
	    {
	      *(pfound->flag) = pfound->val;
	      return 0;
	    }
	  return pfound->val;
	}
      /* Can't find it as a long option.  If this is not getopt_long_only,
	 or the option starts with '--' or is not a valid short
	 option, then it's an error.
	 Otherwise interpret it as a short option.  */
      if (!long_only || argv[optind][1] == '-'
#ifdef GETOPT_COMPAT
	  || argv[optind][0] == '+'
#endif				/* GETOPT_COMPAT */
	  || my_index (optstring, *nextchar) == NULL)
	{
	  if (opterr)
	    {
	      if (argv[optind][1] == '-')
		/* --option */
		fprintf (stderr, "%s: unrecognized option `--%s'\n",
			 argv[0], nextchar);
	      else
		/* +option or -option */
		fprintf (stderr, "%s: unrecognized option `%c%s'\n",
			 argv[0], argv[optind][0], nextchar);
	    }
	  nextchar = (char *) "";
	  optind++;
	  return BAD_OPTION;
	}
    }

  /* Look at and handle the next option-character.  */

  {
    char c = *nextchar++;
    const char *temp = my_index(optstring, c);

    /* Increment `optind' when we start to process its last character.  */
    if (*nextchar == '\0')
      ++optind;

    if (temp == NULL || c == ':')
      {
	if (opterr)
	  {
#if 0
	    if (c < 040 || c >= 0177)
	      fprintf (stderr, "%s: unrecognized option, character code 0%o\n",
		       argv[0], c);
	    else
	      fprintf (stderr, "%s: unrecognized option `-%c'\n", argv[0], c);
#else
	    /* 1003.2 specifies the format of this message.  */
	    fprintf (stderr, "%s: illegal option -- %c\n", argv[0], c);
#endif
	  }
	optopt = c;
	return BAD_OPTION;
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
	      optarg = 0;
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
#if 0
		    fprintf (stderr, "%s: option `-%c' requires an argument\n",
			     argv[0], c);
#else
		    /* 1003.2 specifies the format of this message.  */
		    fprintf (stderr, "%s: option requires an argument -- %c\n",
			     argv[0], c);
#endif
		  }
		optopt = c;
		if (optstring[0] == ':')
		  c = ':';
		else
		  c = BAD_OPTION;
	      }
	    else
	      /* We already incremented `optind' once;
		 increment it again when taking next ARGV-elt as argument.  */
	      optarg = argv[optind++];
	    nextchar = NULL;
	  }
      }
    return c;
  }
}

int getopt(int argc, char * const * argv, const char * optstring)
{
  return _getopt_internal (argc, argv, optstring,
			   (const struct option *) 0,
			   (int *) 0,
			   0);
}

int getopt_long(int argc, char *const *argv, const char *options, const struct option * long_options, int * opt_index)
{
  return _getopt_internal (argc, argv, options, long_options, opt_index, 0);
}

#endif


