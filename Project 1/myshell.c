/**
 * Implements all operation for a shell.
 *
 * @author : Probal chandra dhar
 * @Filename : myshell.c
 * @Date : 01/25/17
 * @course : COP5990
 * @Project # : 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parse.h"
#include "myshell.h"

int main(int argc, char **argv){

    // keeping track of any background process is running. At first setting the value to 0
    int backgroundProcessCounter = 0;

    // If user wants to use debug mode or not
    int debug = 0;

    // Usage
    if ( argc > 2 ){
        printf ("usage: myshell [-Debug]\n");
        exit(0);
    }

    // Usage
    if ( argc == 2 && strcmp( argv[1], "-Debug" ) ){
        printf ("usage: myshell [-Debug]\n");
        exit(1);
    } else if ( argc == 2 && !strcmp( argv[1], "-Debug" ) ) { // setting debug to 1 if user wants the shell in Debug mode
        debug = 1;
    }

    // Declearing a variable of type Param_t structure
    Param_t argumentParams;

    // at beginning set the argumentCount to 0
    argumentParams.argumentCount = 0;

    // Infinite loop
    while (1)
    {

        // Prompt user to enter command  
        printf("$$$ ");

        /* 
         *   Read input line from user and tokenize it
         *   Every token will be inserted in the Param_t structure
        */
        argumentParams = read_and_tokenize(&argumentParams.argumentCount);

        // If nothing was entered
        if (argumentParams.argumentCount == 0)
            // Skip command execution
            continue;

        // check if & is entered in the middle of the command
        if ( !strcmp ( argumentParams.argumentVector[0], "wrong&" ) ) {

            // Reset all values of argumentParams
            resetValues(&argumentParams);
            continue;   // Skip command execution
        
        }

        // Exit condition
        if ( breakCondition(&argumentParams, backgroundProcessCounter) )
            break;

        // Print all params with redirection & background for debugging
        if ( debug ){
            printf("-------------- Debug start --------------\n\n");
            printParams(&argumentParams);
            printf("\n-------------- Debug end ----------------\n\n");
        }

        // assigning NULL to the last element of the argumentVector
        argumentParams.argumentVector[argumentParams.argumentCount] = NULL;

        // Get the return value of executionProcess function
        int funcBgProcessCounter;

        // Execute the command also do the redirection
        funcBgProcessCounter = executionProcess(&argumentParams, backgroundProcessCounter);

        // Store the funcBgProcessCounter to backgroundProcessCounter
        backgroundProcessCounter = funcBgProcessCounter;

        // Reset all values of argumentParams
        resetValues(&argumentParams);
        // Start again
    }


    return 0;

}


/**
 * Resets the values of the param's variables
 *
 * @param param - input all the arguments of the shell command
 *
 */
void resetValues(Param_t *param) {

    // Reset argument array and its index
    int i;
    for ( i = 0; i <= param->argumentCount; i++)
        param->argumentVector[i] = NULL;

    param->inputRedirect = NULL;
    param->outputRedirect = NULL;

    param->argumentCount = 0;

}

/**
 * Execute the command entered by an user also do input/out redirect
 *
 * @param param - input all the arguments of the shell command
 * @param bgProcessCounter - number of background process running
 *
 * @return - number of background process running
 */
int executionProcess(Param_t *param, int bgProcessCounter) {

    int status;

    //fork child
    int pid;
    pid = fork();

    //parent Process
    if ( pid ) {

        if ( param->background == 0 ){

            /**
                Foreground Process
            */

            wait(&status);       /*  wait for completion  */

        } else {

            /**
                Background Process
            */

            // keeping track of the background process
            if ( waitpid(pid, &status, WNOHANG) == 0 )
                bgProcessCounter++;

        }

    // child Process
    } else {

        // Input Redirection
        if ( param->inputRedirect != NULL ) {

            // Checking if the file exists or not
            if ( freopen(param->inputRedirect, "r", stdin) == NULL )
                printf("No file to open\n");

        }

        // Output Redirection
        if ( param->outputRedirect != NULL )
            freopen(param->outputRedirect, "w", stdout);

        // Execute the command
        if ( execvp(param->argumentVector[0], param->argumentVector) == -1 ){
            perror(param->argumentVector[0]);
        }

        // Execution error
        fprintf(stderr, "Can't execute the command\n");
        exit(EXIT_FAILURE);

    } // End Parent Process

    return bgProcessCounter;

}

/**
 * Check if user wants to exit or not
 *
 * @param param - input all the arguments of the shell command
 * @param backgroundProcessCounter - number of background process running
 *
 * @return - 1 if exit is entered, 0 otherwise
 *
 */
int breakCondition( Param_t *param, int backgroundProcessCounter ) {

    // Exit condition
    if ( param->argumentCount != 0 && !strcmp( param->argumentVector[0], "exit" ) ){

        // wait for any background process running
        int i;
        for ( i = 0; i < backgroundProcessCounter; i++ )
        {
            wait(NULL);     // wait for completion
        }

        printf("************* Exiting myshell **************\n");

        return 1;
    }

    return 0;

}




