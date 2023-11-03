#include "parser/ast.h"
#include "shell.h"
#include <stdio.h>

void initialize(void)
{
    /* This code will be called once at startup */
    if (prompt)
        prompt = "vush$ ";
}

void run_command(node_t *node)
{
    /* Print parsed input for testing - comment this when running the tests! */
    print_tree(node);
    if(*node == 'ls')
	printf("hahahaha");
	
    if (prompt)
        prompt = "vush$ ";
}
