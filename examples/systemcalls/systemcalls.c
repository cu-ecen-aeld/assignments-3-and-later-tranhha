#include "systemcalls.h"
#include <sys/wait.h>//for WIFEXITED and WEXITSTATUS macros
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/	
	int status = system(cmd);
	//check if system() is called successfully
	if (status == -1)
		return false;
    	
	//check if the command was executed successfully
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		   return true;
	else
		   return false;
				
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
	va_list args;// Declare a variable to hold the argument list
    	va_start(args, count);// Initialize the argument list, starting after 'count'
    	char * command[count+1];
    	int i;
    	for(i=0; i<count; i++)
    	{
     	command[i] = va_arg(args, char *);// Retrieve each argument as a char* and store it in the command arraiy
    	}
   	command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    	command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
	//Ensure the command has an absolute path
	if (command[0][0] != '/') { perror("path is not absolute path"); return false; }

	pid_t pid = fork(); // Fork the process to create a child for execv
	if (pid < 0)
	{
		   perror("fork failed\n");
		   exit(1);
	}

	//Child process
	else if (pid == 0)
	{
		printf("Child process (PID:%d) is running.\n", pid);
		int status = execv(command[0], command); //execute command with execv
		if (status == -1)
		{
			   perror("execv failed.\n");
			   return false;
		}

	}
	
	//Parent process
	else
	{
		printf("Parent process is running.\n");
		int wstatus;
		int status = waitpid(pid, &wstatus, 0); //wait for child process to finish
		if (status == -1)
		{
			   perror("waitpid failed.\n");
			   return false;
		}

		//check if command executed in child exited successfully
		if(WIFEXITED(wstatus) && WEXITSTATUS(wstatus)==0)
		{
			   printf("do_exec success.\n");
			   return true;
		}
		else
		{
			   perror("command failed");
			   return false;
		}	   
	}
    va_end(args); //clean up argument list

    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    	//Open the output file for writing
    	int fd = open(outputfile, O_WRONLY|O_CREAT, 644);
	if (fd < 0) { perror("failed to open file"); return false;}

	pid_t pid = fork(); //fork the process to create a child for execv
	if (pid < 0)
	{
		   perror("fork failed\n");
		   exit(1);
	}

	//CHild process
	else if (pid == 0)
	{
		printf("Child process (PID:%d) is running.\n", pid);
		
		//Redirect stdout to outputfile referred by fd
		if(dup2(fd, 1) < 0) { perror("dup2 failed"); return false; }

		int status = execv(command[0], command);
		if (status == -1)
		{
			   perror("execv failed.\n");
			   return false;
		}

	}
	
	//Parent process
	else
	{
		printf("Parent process is running.\n");
		int wstatus;
		int status = waitpid(pid, &wstatus, 0);//wait for child process to finish
		if (status == -1)
		{
			   perror("waitpid failed.\n");
			   return false;
		}

		// Check if command executed in child process exited successfully
		if(WIFEXITED(wstatus) && WEXITSTATUS(wstatus)==0)
		{
			   printf("do_exec success.\n");
			   return true;
		}
		else
		{
			   perror("command failed");
			   return false;
		}	   

	}
    va_end(args);

    return true;
}
