/**
 * Defines all operation for a parser
 *
 * @author : Probal chandra dhar
 * @Filename : parse.h
 * @Date : 01/25/17
 * @course : COP5990
 * @Project # : 1
 */

#ifndef _PARSE_H
#define _PARSE_H

/* don’t test program with more than this many tokens for input */
#define MAXARGS 32
/* structure to hold input data */
struct PARAM
{
    char *inputRedirect;            /* file name or NULL */
    char *outputRedirect;           /* file name or NULL */
    int  background;                /* either 0 (false) or 1 (true) */
    int  argumentCount;             /* number of tokens in argument vector */
    char *argumentVector[MAXARGS]; /* array of strings */
};

/* a typedef so we don’t need to use "struct PARAM" all the time */
typedef struct PARAM Param_t;

/**
 * Print all the params for a shell
 *
 * @param param - input all the arguments of the shell command
 *
 */
void printParams(Param_t * param);

/** 
 *
 * Function that reads a line from user and processes it by tokenizing the line 
 * using delimiters, and returns a new array of arguments composed of each string token
 *
 * @param argIndex - argumentCount of the new command
 * @return all the tokenized argument in a Param_t format
 *
*/
Param_t read_and_tokenize(int *argIndex);


#endif
