/**
 * Defines all operation for myshell
 *
 * @author : Probal chandra dhar
 * @Filename : myshell.h
 * @Date : 01/25/17
 * @course : COP5990
 * @Project # : 1
 */

#ifndef _MYSHELL_H
#define _MYSHELL_H

/**
 * Execute the command entered by an user also do input/out redirect
 *
 * @param param - input all the arguments of the shell command
 * @param bgProcessCounter - number of background process running
 *
 * @return - number of background process running
 */
int executionProcess(Param_t *param, int bgProcessCounter);

/**
 * Resets the values of the param's variables
 *
 * @param param - input all the arguments of the shell command
 *
 */
void resetValues(Param_t *param);


/**
 * Check if user wants to exit or not
 *
 * @param param - input all the arguments of the shell command
 * @param backgroundProcessCounter - number of background process running
 *
 * @return - 1 if exit is entered, 0 otherwise
 *
 */
int breakCondition(Param_t *param, int backgroundProcessCounter);












#endif