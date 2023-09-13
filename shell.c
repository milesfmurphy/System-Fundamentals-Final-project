// PID: 730522786
// I pledge the COMP211 honor code.

// ----------------------------------------------
// These are the only libraries that can be
// used. Under no circumstances can additional
// libraries be included

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shell.h"

const char *PATH_SEPARATOR = ":";

// --------------------------------------------
// Currently only two builtin commands for this
// assignment exist
// --------------------------------------------
const char* BUILT_IN_COMMANDS[] = { "cd", "exit", NULL };


/* ------------------------------------------------------------------------------
 *
 * YOU NEED TO COMPLETE THIS FUNCTION.
 *
 * Allocate memory for the command. The last element in `p_cmd->argv` should be
 * NULL.
 *
 * The length of `p_cmd->argv` should be `argc+1`: the first `argc` slots are
 * used to store the arguments and the last one is set to NULL.
 *
 * Arguments:
 *      p_cmd : pointer to the command need to allocate memory.
 *      argc :  the number of arguments.
 *
 * Return:
 *      None
 */
void alloc_mem_for_command(command_t* p_cmd, int argc) {
    int i = 0;
    p_cmd->argv = malloc(sizeof (char*) * (argc+1));
    while(i < argc){
        p_cmd->argv[i] = malloc(sizeof(char)*(MAX_ARG_LEN));
        i++;
    }
    p_cmd->argc = argc;
    p_cmd->argv[argc] = NULL;
} // end alloc_mem_for_command function

void cleanup(command_t* p_cmd) {
    int i = 0;
    while(i< p_cmd->argc){
        free(p_cmd->argv[i]);
        p_cmd->argv[i] = NULL;
        i++;
    }
    free(p_cmd->argv);
    p_cmd->argv = NULL;
} // end cleanup function

void parse(char* line, command_t* p_cmd) {
    int argcount = 0;
    int flag = 0;
    for(int i = 0; line[i]!= '\0'; i++){
        if(line[i] != ' '&& flag == 0){
            argcount++;
            flag = 1;
        }
        else if (line[i] == ' '){
            flag = 0;
        }
        else{
            flag = 1;
        }
    }
    alloc_mem_for_command(p_cmd, argcount);

    char path[MAX_ARG_LEN];
    strcpy(path, line);
    char * lineSegment = strtok(path, " ");
    for(int i = 0; lineSegment != NULL; lineSegment = strtok(NULL, " "), i++){
        strcpy(p_cmd->argv[i], lineSegment);
    }
    p_cmd->argc = argcount;
} // end parse function

int find_fullpath( command_t* p_cmd ) {
    char *path_env_variable = getenv("PATH");
    char path[MAX_ARG_LEN]; //Might need to turn path into non-char pointer
    strcpy(path, path_env_variable);
    char *parsed = strtok(path, PATH_SEPARATOR);
    while (parsed != NULL) {
//Make the strings, combine them, and make SURE they are normal strings, not pointers
        struct stat buff;
        char string[MAX_ARG_LEN];
        strcpy(string, parsed);
        strcat(string, "/"); //Have to add slash, so you can add our main argument after
        strcat(string, p_cmd->argv[0]);
//Copy layout of stat from git
        if (stat(string, &buff) == 0 && (S_IFREG & buff.st_mode)) {
            //I file named echo is in the /usr/local/sbin folder
            strcpy(p_cmd->argv[0], string); //maybe have to copy parsed into a normal string
            return TRUE;
        }
        parsed = strtok(NULL, PATH_SEPARATOR);
    }
    return FALSE; // Default return value
}

int execute( command_t* p_cmd ) {
    int status = SUCCESS;
    int child_process_status;
    pid_t child_pid;
    if(is_builtin(p_cmd) == TRUE){
        do_builtin(p_cmd);
        return status;
    }
    if(find_fullpath(p_cmd) == TRUE){
            child_pid = fork();
            if(child_pid == 0){
                execv(p_cmd->argv[0], p_cmd->argv);
                status = SUCCESS;
            }
            if(child_pid>0){
                wait(&child_process_status);
            }
            if(child_pid<0){
                status = ERROR;
            }
        }
        else {
            printf("Command '%s' not found!\n", p_cmd->argv[0]);
            status = ERROR;
        }
    return status;
    }
    // TODO: COMPLETE YOUR CODE HERE.

 // end execute function

/* ------------------------------------------------------------------------------
 *
 * This function will determine if command (cmd for short) entered in the shell
 * by the user is a valid builtin command.
 *
 * HINT(s): Use BUILT_IN_COMMANDS array defined in shell.c
 *
 * Arguments:
 *      p_cmd: pointer to the command_t structure
 * Return:
 *      TRUE:  the cmd is in array `valid_builtin_commands`.
 *      FALSE: not in array `valid_builtin_commands`.
 *
 */
int is_builtin(command_t* p_cmd) {

    int cnt = 0;

    while ( BUILT_IN_COMMANDS[cnt] != NULL ) {

        if ( strcmp( p_cmd->argv[0], BUILT_IN_COMMANDS[cnt] ) == 0 ) {
            return TRUE;
        }

        cnt++;
    }

    return FALSE;

} // end is_builtin function

/* ------------------------------------------------------------------------------
 *
 * This function is used execute built-in commands such as change directory (cd)
 *
 * HINT(s): See man page for more information about chdir function
 *
 * Arguments:
 *      p_cmd: pointer to the command_t structure
 * Return:
 *      SUCCESS: no error occurred during chdir operation.
 *      ERROR: an error occured during chdir operation.
 *
 */
int do_builtin(command_t* p_cmd) {
    // If the command was 'cd', then change directories
    // otherwise, tell the program to EXIT

    struct stat buff;
    int status = SUCCESS;

    // exit
    if (strcmp( p_cmd->argv[0], "exit") == 0) {
        exit(status);
    }

    // cd
    if (p_cmd->argc == 1) {
        // -----------------------
        // cd with no arg
        // -----------------------
        // change working directory to that
        // specified in HOME environmental
        // variable

        status = chdir(getenv("HOME"));
    } else if ( (status = stat(p_cmd->argv[1], &buff)) == 0 && (S_IFDIR & buff.st_mode) ) {
        // -----------------------
        // cd with one arg
        // -----------------------
        // only perform this operation if the requested
        // folder exists

        status = chdir(p_cmd->argv[1]);
    }

    return status;

} // end do_builtin function
