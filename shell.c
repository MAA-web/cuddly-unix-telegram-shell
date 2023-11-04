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

void ls(const char*);
void c_uname();
void c_echo(size_t, char**);
void cd(size_t, char**);
void c_sleep(size_t, char**);
void c_mkdir(size_t, char**);
void c_exit(size_t, char**);

char* current_working_directory;
char* new_directory;

void initialize(void)
{
    //new_directory = malloc(1);
    new_directory = getenv("PWD");
    current_working_directory = malloc(strlen(new_directory) + 1); // +1 for the null terminator
    if (current_working_directory) {
        strcpy(current_working_directory, new_directory);
    }

    /* This code will be called once at startup */
    if (prompt){
        prompt = (char*)malloc(256);
        sprintf(prompt, "unix_shell %s $ ", current_working_directory);
    }

}

void run_command(node_t *node)
{
    /* Print parsed input for testing - comment this when running the tests! */
    //print_tree(node);

    if (node->type == NODE_COMMAND) {

        
        if (strcmp(node->command.program, "_ls") == 0) {
        
            //printf("Executing 'ls' command\n");

            if (node->command.argc > 1) {

                ls(node->command.argv[1]);
            
            } else {
            
                ls(".");
            
            }
            printf("\n");

        } else if (strcmp(node->command.program, "uname") == 0)
        {
            c_uname();
            
        }
        else if (strcmp(node->command.program, "mkdir") == 0){

            c_mkdir(node->command.argc, node->command.argv);
        
        }
        else if (strcmp(node->command.program, "echo") == 0)
        {
            
            c_echo(node->command.argc, node->command.argv);

        } 
        else if (strcmp(node->command.program, "exit") == 0) {
            
            c_exit(node->command.argc, node->command.argv);
        
        } 
        else if (strcmp(node->command.program, "cd") == 0)
        {
            cd(node->command.argc, node->command.argv);
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
                // Parent process
                int status;
                wait(&status);
                
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) { // Child process executed successfully
                } else {
                    perror("Child process failed");
                }
            }

        }
        
        
        //else {
            
        //    printf("unix_shell: %s: command not found...\n",node->command.program);
            
        //}
    }
    else if (node->type == NODE_SEQUENCE)
    {
        // Handle sequence of commands
        node_t *current_node = node->sequence.first;

        while (current_node != NULL) {
            run_command(current_node);
            current_node = current_node->sequence.second;
        }
        
    }
    
    
	
    if (prompt){
        sprintf(prompt, "unix_shell %s $ ", current_working_directory);
    }
}





void ls(const char *directory) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(directory);
    if (dir == NULL) {
        perror("opendir error: this path coud possibly not be a directory!!");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
}

void c_uname() {
    struct utsname uname_data;
    if (uname(&uname_data) == -1) {
        perror("uname");
        return;
    }

    printf("%s\n", uname_data.sysname);
}

void c_echo(size_t argument_number, char** arguments) {
    
    if (argument_number > 1 && !strcmp(arguments[1], "-n")) {

        printf("%s",arguments[2]);
    
    } else {
        
        printf("%s\n", arguments[1]);
    
    }

}

void cd(size_t argument_number, char** arguments) {
    if (argument_number > 1)
    {
        
        DIR *dir = opendir(arguments[1]);
        if (dir == NULL)
        {
            closedir(dir);
            perror("This directory does not exist or is not a directory");
        }
        else
        {
            closedir(dir);

            if (realpath(arguments[1], new_directory) == NULL) {
                perror("cd error");
            } else {
                if (chdir(new_directory) == 0) {

                    current_working_directory = strdup(new_directory);
                    
                } else {
                    perror("cd error");
                }
            }

        }
    } else
    {
        chdir(getenv("HOME"));
        current_working_directory = getenv("HOME");
    }
    
    
    
    //printf("%s\n",current_working_directory);
}

void c_sleep(size_t argument_number, char** arguments){
    if (argument_number > 1)
    {
        sleep(atoi(arguments[1]));
    }
    
}

void c_mkdir(size_t argument_number, char** arguments) {
    if (argument_number > 1 && argument_number < 2)
    {
        if (mkdir(arguments[1], 0777) == 0) {
            printf("Directory '%s' created.\n", arguments[1]);
        } else {
            perror("mkdir");
        }
    }
    else
    {
        printf("mkdir usage:\n\tmkdir <directory name to be created>");
    }
    
}



void c_exit(size_t argument_number, char** arguments) {
    if (prompt)
    {
        free(prompt);
    }
    
    free(current_working_directory);
    int exit_return = 0;
    if (argument_number > 1)
    {
        exit_return = atoi(arguments[1]);
        exit(exit_return);
    }
    else
    {
        exit(exit_return);
    }
    
    
}