/*
 *  module  :  DOSOPT.H         version  1.10
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


#ifndef _DOSOPT_H_


int getOption(int argc, char *argv[], int optc, char *optv[] );
/*
 *  getOption:  get the index of the next command line option listed in
 *              the option vector.
 *              (search strategie: the longest match)
 *
 *              returns  index of the option in the option vector,
 *                       or EOF, if no more options.
 */


int isOption(int argc, char *argv[], int optc, char *optv[], int nth );
/*
 *  isOption:  proofs, whether the n-th command line argument is an option
 *             listed in the option vector.
 *             (search strategie: the longest match)
 *
 *             returns   1, if the argument is an option
 *                       0, if it is not an option
 *                     EOF, on error
 */


char *getOptionParameter();
/*
 *  getOptionParameter:  returns a pointer to the parameter of the current
 *                       command line option determined by getOption(). The
 *                       pointer may be NULL if there is no parameter or no
 *                       current option.
 *                       Skips a leading equal-sign or colon or enclosing
 *                       quotation marks.
 */


#define _DOSOPT_H_
#endif

