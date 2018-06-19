/*
 * Name: Matthew Toro
 * Class: CS 344 Operating Systems
 * Assignment: Program 3 Small Shell
 * Due Date: 8/7/2017
 * Description: A command line interface (shell) that interacts with the OS
 * */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "dynArray.h"

// Function prototypes
void ASCII_String(char * string);
void getInput(char * buffer, size_t bufferSize);
int findInputRedirection(char ** args, int length);
int findOutputRedirection(char ** args, int length);
int findPIDExpansion(char ** args, int length);
void expandPID(char * str);
int isBackgroundProcess(char * arg);
void catchSIGINT(int signalNumber);
void catchSIGTSTP(int signalNumber);

// Global Constants
const int STD_OUT = 1;
const int STD_IN = 0;
const int STD_ERR = 2;
const int MAX_CHARS = 2048;
const int STR_CMP_TRUE = 0;

// Global variable to turn foreground Mode on or off
int foregroundMode = 0;

// main function starts here
int main()
{
   // initialize input variables
   char * dummyInput = malloc(MAX_CHARS * sizeof(char));
   char * actualInput = malloc(MAX_CHARS * sizeof(char));
   size_t bufferSize = MAX_CHARS;

   int exitStatus = 0;
   DynArr* backgroundPids;
   backgroundPids = newDynArr(10);

   // reference lecture 3.3 Signals
   // completely initialize struct to be empty
   struct sigaction SIGINT_action = {0};
   // block/delay all signals arriving while this mask is in place
   sigfillset(&SIGINT_action.sa_mask);
   // set no flags in the struct
   SIGINT_action.sa_flags = 0;
   // set the action handler to ignore this signal
   SIGINT_action.sa_handler = SIG_IGN;
  
   sigaction(SIGINT, &SIGINT_action, NULL);

   // set up SIGTSTP handler
   struct sigaction SIGTSTP_action;
   SIGTSTP_action.sa_handler = catchSIGTSTP;
   sigfillset(&SIGTSTP_action.sa_mask); 
   SIGTSTP_action.sa_flags = SA_RESTART;

   sigaction(SIGTSTP, &SIGTSTP_action, NULL); 
   
   memset(dummyInput, '\0', sizeof(dummyInput));
   memset(actualInput, '\0', sizeof(actualInput));
   int childExitMethod = -5;

   // begin main loop: loop terminates in child exec or exit OR parent terminates on exit command
   while(1)
   {
	// get input from user into variables
	getInput(dummyInput, bufferSize);
	strcpy(actualInput, dummyInput);

	char * savePtrOne;
	char * token = strtok_r(dummyInput, " \t", &savePtrOne);

	//checks if the ascii character is equal to #(35)
	if (token[0] == 35)
	{
		// if a comment, do nothing
	}
	else if (strcmp(&token[0], "\n") == STR_CMP_TRUE)
	{
		// if a newline, do nothing
		// tokenize the string solves the issue of "  \t\n"
		// i.e. spaces before the return
	}
	else
	{
	   // 2d array used to store the strings from the input command
	   char ** args;
	  
 	   // very important: keeps track of how many strings are in the input command
	   int count = 0;
	
	/* I ran into a weird bug with strtok
	 the token would get valid data until the string ended
	 then it would seg fault but the value of the token was not null
	 I could not print the token to see what it was as it would seg fault
	 so I just count how many time it runs 
	 and run a for loop with a parallel variable below
	*/
	   do
	   {
		token = strtok_r(NULL, " \t", &savePtrOne);
		count++;
	   } while (token);

	   // process the input into the args array
	   // I do (count + 1) to accomodate the ending NULL pointer
	   args = malloc((count + 1) * sizeof(char *));
	   int index;
	   char * savePtr;
	   for (index = 0; index < count; index++)
	   {
		// the \n is added to the delimiter to deal with newlines 
		// from user input
		if (index == 0)
			token = strtok_r(actualInput, " \t\n", &savePtr);
		else
			token = strtok_r(NULL, " \t\n", &savePtr);

		args[index] = malloc((strlen(token) + 1) * sizeof(char));
		memset(args[index], '\0', sizeof(args[index]));
		strcpy(args[index], token);
	   }
	   args[count] = NULL;

	   // expand $$ in commands
	   int pidIndex = findPIDExpansion(args, count + 1);
	   if (pidIndex != -1)
		expandPID(args[pidIndex]);

	// now the program branches off depending on user input
	// check for built in commands first
	// this way we can ignore file redirection and 
	// background process requests
	//
	// the first command is 'cd'
	   if (strcmp(args[0], "cd") == STR_CMP_TRUE)
	   {
		if (count == 1)
		{
		   chdir(getenv("HOME"));
		}
		else
		{
		// I support absolute paths by appending the home path to the command 
		   if (args[1][0] == 47)
		   {
		      char dirName[100];
		      memset(dirName, '\0', sizeof(dirName));
		      strcat(dirName, getenv("HOME"));
		      strcat(dirName, args[1]);
		      chdir(dirName);	
		   }
		   else
		   {		
		      chdir(args[1]);
		   }
		}
	   }
	   else if (strcmp(args[0], "status") == STR_CMP_TRUE)
	   {
		// 'status' command
		// create an exit value string and print it out
		char exitStatusString[2];
		snprintf(exitStatusString, 2, "%d", exitStatus);
		char statusPrompt[15];
		memset(statusPrompt, '\0', sizeof(statusPrompt));
		strcpy(statusPrompt, "exit value ");
		strcat(statusPrompt, exitStatusString);
		strcat(statusPrompt, "\n");
		write(STD_OUT, statusPrompt, strlen(statusPrompt));
		fflush(stdout);
	   }
	   else if (strcmp(args[0], "exit") == STR_CMP_TRUE)
	   {
		// 'exit' command
		// iterate through any still running background processes and terminate them
	   	int m;
	   	int bgProcessPID;
		int bgExitStatus;
	   	for (m = 0; m < sizeDynArr(backgroundPids); m++)
	   	{
		   bgProcessPID = waitpid(getDynArr(backgroundPids, m), &childExitMethod, WNOHANG);
		   if (bgProcessPID == 0)
		   {
			kill(bgProcessPID, SIGTERM);
			removeAtDynArr(backgroundPids, m);
			break;	
		   }		
	   	}
		// free input memory and break from the original while loop
	   	int j;
	   	for (j = 0; j < count; j++)
	   	{
	      	   free(args[j]);
	   	}
	   	free(args);
		break;
	   }
	   else
	   {
		// this section handles non-built-in commands

		// see if there is an & in last argument
		int backgroundProcess = isBackgroundProcess(args[count - 1]);

		if (backgroundProcess == 1 && foregroundMode == 0)
		{
		    free(args[count - 1]);
		    args[count - 1] = NULL;
		}
		else if (backgroundProcess == 1 && foregroundMode == 1)
		{
		    free(args[count - 1]);
		    args[count - 1] = NULL;
		}

		pid_t spawnPid = -5;
 		int childPID_actual;

		spawnPid = fork();
		// since we fork here, a child will return 0
		// and the parent will be the ID of the child process  
		// which we catch in a default statement below
		
		switch (spawnPid) 
		{
		   case -1:
		   {
			// fork failed, no child process is created
			write(STD_OUT, "fork failed\n", 12);
			fflush(stdout);
			break;
		   }	
		   case 0:
		   {
			// child process executes this code
			//
			// IMPORTANT: sets SIGINT handler to default behavior for child only
			// if the process is not a background process or the foreground mode is on
			if (backgroundProcess != 1  || foregroundMode == 1)
			{
			   SIGINT_action.sa_handler = SIG_DFL;
			   sigaction(SIGINT, &SIGINT_action, NULL);
			}

			int inputFD;
			int outputFD;
			
			// handle input files
			int inputIndex = findInputRedirection(args, count+1);
			if (inputIndex != -1)
			{
			   // verify the file
			   if (args[inputIndex + 1] == NULL)
			   {
			   	if (backgroundProcess == 1 && foregroundMode == 0)
			   	{
				   // redirect to /dev/null if no input file specified
				   int devNull = open("/dev/null", O_WRONLY);
				   if (devNull != -1)
				   {
					dup2(STD_IN, devNull);
				   }
			  	}
				else
				{
				   write(STD_OUT, "No file name provided\n", 22);
				   fflush(stdout);
				   exit(1);
				}
			   }
			   else
			   {
				// attempt to open it, read only
				inputFD = open(args[inputIndex + 1], O_RDONLY);
				if (inputFD == -1)
				{
			   	   // if fails, print error and exit(1)
				   write(STD_OUT, "File could not be read\n", 23);
				   fflush(stdout);
				   exit(1);
				}
				else
				{
			   	   // else, dup2 the file descripter 
			  	   // free those strings, and set them to NULL
				   dup2(inputFD, STD_IN);
				   free(args[inputIndex]);
				   free(args[inputIndex + 1]);
				   args[inputIndex] = NULL;
				   args[inputIndex + 1] = NULL;
				}
			   }		
			}
		
			// handle output files	
			int outputIndex = findOutputRedirection(args, count+1);
			if (outputIndex != -1)
			{
			   // verify the file
			   if (args[outputIndex + 1] == NULL)
			   {
			   	if (backgroundProcess == 1 && foregroundMode == 0)
			   	{
				   // redirect to /dev/null if no input file specified
				   int devNull = open("/dev/null", O_WRONLY);
				   if (devNull != -1)
				   {
					dup2(STD_IN, devNull);
				   }
			  	}
				else
				{
				   write(STD_OUT, "No file name provided\n", 22);
				   fflush(stdout);
				   exit(1);
				}
			   }
			   else
			   {
				// attempt to open it, write only
				outputFD = open(args[outputIndex + 1], 
				O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC,
				S_IRUSR | S_IWUSR);
				
				if (outputFD == -1)
				{
			   	   // if fails, print error and exit(1)
				   write(STD_OUT, "File could not be opened\n", 25);
				   fflush(stdout);
				   exit(1);
				}
				else
				{
			   	   // else, dup2 the file descripter 
			  	   // free those strings, and set them to NULL
				   dup2(outputFD, STD_OUT);
				   free(args[outputIndex]);
				   free(args[outputIndex + 1]);
				   args[outputIndex] = NULL;
				   args[outputIndex + 1] = NULL;
				}
			   }		
			}
	
			// child termination is handled by exec
			// or if it fails, exit(1) 
			// from my understanding, exec or exit will handle 
			// memory leaks in the child process 

			int execStatus = 0;
			execStatus = execvp(args[0], args);
			if (execStatus == -1)
			{
			   write(STD_OUT, 
			   "error: command could not run\n", 29);
			   fflush(stdout);
			}

			exit(1);
		   }
		   default:
		   {
			// parent executes this code
		
			if (backgroundProcess == 1 && foregroundMode == 0)
			{
			   // do not wait on child (WNOHANG)
			   childPID_actual = waitpid(spawnPid, &childExitMethod, WNOHANG); 

			   // create and write background pid is ##	
			   char backgroundPIDStr[8];
			   memset(backgroundPIDStr, '\0', sizeof(backgroundPIDStr));
			   int childPid = spawnPid;
			   sprintf(backgroundPIDStr, "%d", childPid);
			   char backgroundPrompt[30];
			   memset(backgroundPrompt, '\0', sizeof(backgroundPrompt));
			   strcpy(backgroundPrompt, "background pid is ");
			   strcat(backgroundPrompt, backgroundPIDStr);
			   strcat(backgroundPrompt, "\n");

			   write(STD_OUT, backgroundPrompt, strlen(backgroundPrompt));
			   fflush(stdout);

			   // add pid to array
			   addDynArr(backgroundPids, childPid);
			   // background processes do not set the exit status
			}
			else
			{
			   // block the parent until specified child terminates
			   childPID_actual = waitpid(spawnPid, 
			   &childExitMethod, 0);
			   if (childPID_actual == -1)
			   {
			      write(STD_OUT, "wait failed\n", 12);
			      fflush(stdout);
			   }
			   // check and set the exit status
			   if (WIFEXITED(childExitMethod) != 0)
			   {
			      exitStatus = WEXITSTATUS(childExitMethod);
			   }
			   else if (WIFSIGNALED(childExitMethod) != 0)
			   {
			      exitStatus = WTERMSIG(childExitMethod);

			      //write here that the child was terminated by a signal
			      char exitStatusString[4];
			      snprintf(exitStatusString, 4, "%d", exitStatus);
			      char statusPrompt[30];
			      memset(statusPrompt, '\0', sizeof(statusPrompt));
			      strcpy(statusPrompt, "terminated by signal ");
			      strcat(statusPrompt, exitStatusString);
			      strcat(statusPrompt, "\n");
			      write(STD_OUT, statusPrompt, strlen(statusPrompt));
			      fflush(stdout);
			   }	
			}
	
			break;
		   }
		}

	   	// print completed background processes here
	   	// iterate through a PID array checking if waitpid != 0
	   	// if != 0 background pid # is done: exit value #
	   	int m;
	   	int bgProcessPID;
		int bgExitStatus;
	   	for (m = 0; m < sizeDynArr(backgroundPids); m++)
	   	{
		   bgProcessPID = waitpid(getDynArr(backgroundPids, m), &childExitMethod, WNOHANG);
		   if (bgProcessPID != 0)
		   {
			   // set up initial prompt	
			   char backgroundPIDStr[8];
			   memset(backgroundPIDStr, '\0', sizeof(backgroundPIDStr));
			   sprintf(backgroundPIDStr, "%d", bgProcessPID);
			   char backgroundPrompt[70];
			   memset(backgroundPrompt, '\0', sizeof(backgroundPrompt));
			   strcpy(backgroundPrompt, "background pid ");
			   strcat(backgroundPrompt, backgroundPIDStr);
			   strcat(backgroundPrompt, " is done: ");

			   // append exit value, else append signal value
			   if (WIFEXITED(childExitMethod) != 0)
			   {
				bgExitStatus = WEXITSTATUS(childExitMethod);
				strcat(backgroundPrompt, "exit value ");
				char exitStatusString[3];
				snprintf(exitStatusString, 3, "%d", bgExitStatus);
				strcat(backgroundPrompt, exitStatusString);
				strcat(backgroundPrompt, "\n");
			   }
			   else if (WIFSIGNALED(childExitMethod) != 0)
			   {
			        bgExitStatus = WTERMSIG(childExitMethod);
				strcat(backgroundPrompt, "terminated by signal ");
				char exitStatusString[3];
				snprintf(exitStatusString, 3, "%d", bgExitStatus);
				strcat(backgroundPrompt, exitStatusString);
				strcat(backgroundPrompt, "\n");
			   }
			   // print prompt and remove that pid from array	
			   write(STD_OUT, backgroundPrompt, strlen(backgroundPrompt));
			   fflush(stdout);
			   removeAtDynArr(backgroundPids, m);
			   break;	
		   }		
	   	}
		
	   }

	   // free the allocated memory in the parent
	   int j;
	   for (j = 0; j < count; j++)
	   {
	      free(args[j]);
	   }
	   free(args);
	
	}
   }
	// free remaining variables and exit the program
	free(dummyInput);
	free(actualInput);
	freeDynArr(backgroundPids);
	return 0;
}

// function used to get input from user, loops in case a signal interrupts getline
void getInput(char * buffer, size_t bufferSize)
{
   while(1)
   {
	int numCharsEntered;
	write(STD_OUT, ": ", 2); fflush(stdout);
	numCharsEntered = getline(&buffer, &bufferSize, stdin);
	if (numCharsEntered == -1)
		clearerr(stdin);
	else
		break;
   }
}

// function to find '>' in the command arguments 
// returns the index number or -1 if not found
// length = count + 1
int findInputRedirection(char ** args, int length)
{
	int index;
	for (index = 0; index < length; index++)
	{
		if (args[index] != NULL)
		{
			if(strcmp(args[index], "<") == STR_CMP_TRUE)
				return index;
		}
	}
	return -1;	
}


// function to find '<' in the command arguments 
// returns the index number or -1 if not found
// length = count + 1
int findOutputRedirection(char ** args, int length)
{
	int index;
	for (index = 0; index < length; index++)
	{
		if (args[index] != NULL)
		{
			if(strcmp(args[index], ">") == STR_CMP_TRUE)
				return index;
		}
	}
	return -1;	
}

// function to find $$ in the command arguments
// returns index if found or -1 if not found
int findPIDExpansion(char ** args, int length)
{
	int index;
	for (index = 0; index < length; index++)
	{
		if (args[index] != NULL)
		{
			if (strstr(args[index], "$$"))
				return index;
		}
	}
	return -1;	
}

// function that takes the command argument with $$
// and replaces the $$ with the correct pid
void expandPID(char * str)
{
	int pid = getpid();
	char pidStr[10];
	sprintf(pidStr, "%d", pid);
	int pidStrLength = strlen(pidStr);
	int currentStrLength = strlen(str);

	if(currentStrLength == 2)
	{
		// handles cases where str = $$
		free(str);
		str = malloc((pidStrLength + 1) * sizeof(char));
		memset(str, '\0', sizeof(str));
		strcpy(str, pidStr);
	}
	else
	{
		// handles cases where str = word$$
		int newStrLength = pidStrLength + currentStrLength - 2;
		char * tempStr = malloc(newStrLength * sizeof(char));
		memset(tempStr, '\0', sizeof(tempStr));
		strncpy(tempStr, str, strcspn(str, "$$"));
		strcat(tempStr, pidStr);
		free(str);
		str = malloc((newStrLength + 1) * sizeof(char));
		memset(str, '\0', sizeof(str));
		strcpy(str, tempStr);
		free(tempStr);
	}
}

//function that checks whether the passed in command is a background process via '&'
int isBackgroundProcess(char * arg)
{
	if (strcmp(arg, "&") == 0)
		return 1;
	else
		return 0;
}

// switch foreground Mode on or off when this signal is caught
void catchSIGTSTP(int signalNumber)
{
	if (foregroundMode == 0)
	{
		write(STD_OUT, "\nEntering foreground-only mode (& is now ignored)\n: ", 52);
		fflush(stdout);
		foregroundMode = 1;
	}
	else
	{
		write(STD_OUT, "\nExiting foreground-only mode\n: ", 32);
		fflush(stdout);
		foregroundMode = 0;
	}
}
