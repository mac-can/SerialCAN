/*
 *  module  :  DOSOPT.C         version  1.10
 *
 *  purpose :  Get command line option, DOS-style.
 *
 *  export  :  int   getOption(int, char*, int, char*);
 *             int    isOption(int, char*, int, char*, int);
 *             char *getOptionParameter();
 *
 *  include :  usr\dosopt.h
 *
 *  author  :  Uwe Vogt, Berlin.
 *
 *  date    :   8/14/91, 8/22/91
 */

#include <stdio.h>
#include <ctype.h>

#include "dosopt.h"

/*  ---  prototypes  ---
 */

static int optcmp( const char *opt, const char *arg );
static int optlen( const char *opt );


/*  ---  variables  ---
 */

static int   optindex = 0;
static char *optparam = NULL;


/*  ---  public functions  ---
 */

int getOption(int argc, char *argv[], int optc, char *optv[] )
{
  int i, found = EOF;

  optindex += 1;
  optparam = NULL;

  while( optindex < argc )
  {
    for( i = 0; i < optc; i++ )
    {
      if( optcmp( optv[i], argv[optindex] ) == 1 )
	if( (found == EOF) || (optlen( optv[found] ) < optlen( optv[i] )) )
	  found = i;
    }
    if( found != EOF )
    {
      optparam = argv[optindex] + optlen( optv[found] ) + 1;
      if( (*optparam == '=') || (*optparam == ':') )
	optparam++;
      if( *optparam == '\0' )
	optparam = NULL;
      return found;
    }
    else
      optindex += 1;
  }
  return EOF;
}

int isOption(int argc, char *argv[], int optc, char *optv[], int nth )
{
  int i, found = EOF;

  if( (0 < nth) && (nth < argc) )
  {
    for( i = 0; i < optc; i++ )
    {
      if( optcmp( optv[i], argv[nth] ) == 1 )
	if( (found == EOF) || (optlen( optv[found] ) < optlen( optv[i] )) )
	  found = i;
    }
    if( found != EOF )
      return 1;
    else
      return 0;
  }
  else
    return EOF;
}

char *getOptionParameter()
{
  return optparam;
}


/*  ---  local functions  ---
 */

static int optcmp( const char *opt, const char *arg )
{
  int i;

  if( (opt != NULL) && (arg != NULL) )
  {
    if( arg[0] == '/' )
    {
      for( i = 0; i < optlen( opt ); i++ )
      {
	if( arg[i+1] == '\0' )
	  return 0;
	if( toupper( opt[i] ) != toupper( arg[i+1] ) )
	  return 0;
      }
      return 1;
    }
    else
      return EOF;
  }
  else
    return EOF;
}

static int optlen( const char *opt )
/*
 *  optlen:  returns the length of the option string.
 */
{
  int i;

  if( opt != NULL )
  {
    i = 0;
    while( opt[i] != '\0' )
      i += 1;
    return i;
  }
  else
    return EOF;
}

