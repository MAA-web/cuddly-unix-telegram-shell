// Including the necessary libraries
#include "parser/ast.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

// Function Declaraions c_ means custom in this case
void c_uname(void);
void c_echo(size_t, char**);
void c_cd(size_t, char**);
void c_sleep(size_t, char**);
void c_mkdir(size_t, char**);
void c_exit(size_t, char**);
void c_sigint_handler(int);

// Some Global variables
char* current_working_directory;
char* new_directory;



void initialize(void)
{

    new_directory = getenv("PWD");                                  // making new directory equal to the working directory of the environment

    current_working_directory = malloc(strlen(new_directory) + 1); // allocating memory, +1 for the null terminator

    if (current_working_directory) {
        strcpy(current_working_directory, new_directory);           // copying current working directory into the new_directory variable
    }

    signal(SIGINT, c_sigint_handler);                               // setting up the ctrl-c signal handler

    if (prompt){
        prompt = (char*)malloc(256);
        sprintf(prompt, "unix_shell %s $ ", current_working_directory); // storing data in the prompt to be printed
    }

}



void run_command(node_t *node)
{

    if (node->type == NODE_COMMAND) {                               // Handling commands

        // Here if the command matches any of the given names the respective function is called

        if (strcmp(node->command.program, "uname") == 0)            // uname
        {
            c_uname();
            
        }
        else if (strcmp(node->command.program, "mkdir") == 0){      // mkdir

            c_mkdir(node->command.argc, node->command.argv);
        
        }
        else if (strcmp(node->command.program, "echo") == 0)        // echo
        {
            
            c_echo(node->command.argc, node->command.argv);

        } 
        else if (strcmp(node->command.program, "exit") == 0) {      // exit
            
            c_exit(node->command.argc, node->command.argv);
        
        } 
        else if (strcmp(node->command.program, "cd") == 0)          // cd
        {
            c_cd(node->command.argc, node->command.argv);
        }
        else if (strcmp(node->command.program, "sleep") == 0)       // sleep
        {
            c_sleep(node->command.argc, node->command.argv);
        }
        else
        {
            // using execvp when the command does not match any of the already implimented commands

            pid_t pid = fork();

            if (pid < 0) {

                perror("Fork failed");
            
            } else if (pid == 0) {
                // Child process

                    execvp(node->command.argv[0], node->command.argv);
                    perror("execvp failed");
                
                
                
            } else {

                // waiting for the Parent process
                int status;
                wait(&status);
                
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {            // Child process executed successfully
                } else {
                    perror("Child process failed");
                }
            }

        }

    }
    else if (node->type == NODE_SEQUENCE)               // Handling a sequence of commands
    {

        // resursively running the first and second command

        run_command(node->sequence.first);              
        run_command(node->sequence.second);
        
    }
	

    if (prompt){
        sprintf(prompt, "unix_shell %s $ ", current_working_directory);     // Updating directory just in case
    }
}


void c_uname(void) {                        // implementing uname: this command gives data of the system

    struct utsname uname_data;

    if (uname(&uname_data) == -1) {         // checking if uname works
        perror("uname");
        return;
    }

    printf("%s\n", uname_data.sysname);     // Printing just the system name
}

void c_echo(size_t argument_number, char** arguments) {     // Implementing echo
    
    if (argument_number > 1 && !strcmp(arguments[1], "-n")) {   //If -n flag is present no new line is printed

        printf("%s",arguments[2]);
    
    } else {                                                    // otherwise the given argument and a new line are printed
        
        printf("%s\n", arguments[1]);
    
    }

}

void c_cd(size_t argument_number, char** arguments) {             // implementing cd i.e change directory

    if (argument_number > 1)
    {
        
        DIR *dir = opendir(arguments[1]);                       // making pointer of type DIR i.e it holds an open directory

        if (dir == NULL)                                        // if it returns NULL then no directory present and so no directory was opened so, exiting after printing error
        {
            closedir(dir);                                      // closing directory just in case
            perror("This directory does not exist or is not a directory");
            return;
        }
        else                                                    
        {                                                       
            closedir(dir);

            if (realpath(arguments[1], new_directory) == NULL) {    // else we copy the relative path into the new_drectory

                perror("cd error");
            
            } else {
            
                if (chdir(new_directory) == 0) {                        // change the directory to new_directory
                    
                    free(current_working_directory);                    // free the current_working_directory memory buffer
                    
                    current_working_directory = strdup(new_directory);  // making a duplicate and storing it in current working directory
                    
                } else {
                    perror("cd error");
                }
            }

        }
    } else                                                              // If no directory is specified then we will default onto what bash does i.e change directory to the home directory from the environment variables
    {
        chdir(getenv("HOME"));

        free(current_working_directory);                                // We do the same free the buffer

        current_working_directory = getenv("HOME");                     // save the HOME environment variable i.e the home directory to the current_working_directory
    }
    
}

void c_sleep(size_t argument_number, char** arguments){                 // implementinf sleep

    if (argument_number > 1)
    {
        sleep(atoi(arguments[1]));                                      // sleep for a given number of seconds

    }
    
}

void c_mkdir(size_t argument_number, char** arguments) {                // mkdir

    if (argument_number > 1)
    {
        if (mkdir(arguments[1], 0777) == 0) {                           // making a new directory using the system call mkdir the argument[1] is the directory name, 0777 is the access specifier in this case the owner has full access, read and write
            printf("Directory '%s' created.\n", arguments[1]);
        } else {
            perror("mkdir");
        }
    }
    else
    {
        printf("mkdir usage:\n\tmkdir <directory name to be created>"); // printing help if mkdir fails or is wrongly used
    }
    
}

void c_sigint_handler(int signal_number) {                              // function that will handle ctrl-c
    
    if (signal_number == SIGINT)
    {
        rl_on_new_line();                       // Moving to a new line after Ctrl+C to print prompt on new line
        rl_replace_line("", 0);                 // Clearing the current input line
        printf("\n");                           // printing new line
        rl_redisplay();                         // rediaplay the input
    }
}


void c_exit(size_t argument_number, char** arguments) {             // implementing the exit function

    if (prompt)
    {
        free(prompt);                                               // freeing memory buffer of prompt
    }
    
    free(current_working_directory);                                // freeing current_working_directory

    int exit_return = 0;                                            // making exit_return 0

    if (argument_number > 1)                                        // if there is another argument
    {
        exit_return = atoi(arguments[1]);                           // then we return with that number as the exit code
        exit(exit_return);
    }
    else
    {
        exit(exit_return);                                          // otherwise we exit with 0
    }
    
}