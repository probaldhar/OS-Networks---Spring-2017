/**
 * Implements all operation for a parser
 *
 * @author : Probal chandra dhar
 * @Filename : parse.c
 * @Date : 01/25/17
 * @course : COP5990
 * @Project # : 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

#define MAXLINE 2500

/**
 * Print all the params for a shell
 *
 * @param param - input all the arguments of the shell command
 *
 */
void printParams(Param_t * param)
{

	int i;

    printf ("InputRedirect: [%s]\n", (param->inputRedirect != NULL) ? param->inputRedirect:"NULL");
    printf ("OutputRedirect: [%s]\n", (param->outputRedirect != NULL) ? param->outputRedirect:"NULL");
    printf ("Background: [%d]\n", param->background);
    printf ("ArgumentCount: [%d]\n", param->argumentCount);
    
    for (i = 0; i < param->argumentCount; i++)
           printf("ArgumentVector[%2d]: [%s]\n", i, param->argumentVector[i]);

}

/** 
 *
 * Function that reads a line from user and processes it by tokenizing the line 
 * using delimiters, and returns a new array of arguments composed of each string token
 *
 * @param argIndex - argumentCount of the new command
 * @return all the tokenized argument in a Param_t format
 *
*/
Param_t read_and_tokenize(int *argIndex)
{
	// delimiters to tokenize the argument string
    char delimiters[] = " \t\n";
    
    // assigning the token to NULL at beginning of every command to execute
    char *token = NULL;

    // Empty string to get the command
    char *inputLine = (char*) malloc(MAXLINE * sizeof(char));
    
    // args to store the tokenized argument
    char **args = (char**) malloc(MAXARGS * sizeof(char*));

    // Param_t type structure variable
    Param_t allParams;

    // Assigning Param_t variables to NULL or 0
    allParams.argumentCount = 0;
    allParams.background = 0;
    allParams.inputRedirect = NULL;
    allParams.outputRedirect = NULL;

    // reading the input command 
    fgets(inputLine, MAXLINE, stdin);
    
    // tokenize the input command
    token = strtok(inputLine, delimiters);

    // basic condition if token is NULL
    if (token == NULL)
        return allParams;

    // Tokenize line into arguments
    while (token != NULL)
    {
        args[*argIndex] = token;
        token = strtok(NULL, delimiters);
        (*argIndex)++;
    }
    // Add final NULL element
    args[*argIndex] = NULL;

    // Checking all tokenized arguments
    int i;
    for ( i = 0; i < *argIndex; i++ )
    {

    	// assigning NULL character to the string 
	    char tempString[10] = "\0";

	    // storing the first character
        tempString[0] = args[i][0];

        // checking for Input redirection character
        if ( strcmp( tempString, "<") == 0 ) {

        	// storing the filename
            char *inputToken;                
            inputToken = strtok(args[i], "<");
            
            // saving the filename of Param_t structure
            allParams.inputRedirect = inputToken;

        // checking for Output redirection character
        } else if ( strcmp( tempString, ">") == 0 ) {

        	// storing the filename
            char *outputToken;                  
            outputToken = strtok(args[i], ">");

            // saving the filename of Param_t structure
            allParams.outputRedirect = outputToken;

        // checking for background process
        } else if ( strcmp( args[i], "&") == 0 ){

        	// check if & is entered in the middle of the command
        	if ( i != *argIndex - 1 ) {
        		printf("& must always appear as the last character of the command.\n");
        		allParams.argumentVector[0] = "wrong&";
        		return allParams;
        	}

        	// if background set the background to 1
            allParams.background = 1;
        
        // No special argument then save those to argumentVector array
        } else {

        	// saving the arguments and incrementing the argumentCount
            allParams.argumentVector[allParams.argumentCount] = args[i];
            allParams.argumentCount++;
        }
    }

    // returning the Param_t structure
    return allParams;
}


