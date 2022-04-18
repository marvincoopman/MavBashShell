// The MIT License (MIT)
//
// Copyright (c) 2016 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


// Name: Marvin Coopman
// ID:   1001781933
// Spring 2022 3/2/22
// Mav Bash Shell


#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports four arguments

#define MAX_RECORDED_COMMANDS 15 //Mav shell only holds the past 15 commands


struct command
{
  char *command[MAX_NUM_ARGUMENTS];
  int numOfArgs;
  pid_t pid;
  struct command* next;
};

struct command *head = NULL;
struct command *sentinel = NULL;

int historyCount = 0;
//is true if command history is full

void addCommandToHistory(char *token[], int numOfArgs);

//calls print command for all
void printHistory();

void printPidHistory();
//prints out the selected command to terminal
void printCommand(struct command *selectedCommand);

//finds command and puts the command string in token
void rerunNPid(char* token[MAX_NUM_ARGUMENTS],int n);

void updatePidHistory(int pid);

int main()
{

  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");


    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );


    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;

    char *working_string  = strdup( command_string );

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {

      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    if(token[0] == NULL)
    {
      continue;
    }



    //rerunning a command
    if(token[0][0] == '!')
    {
      char *workingToken;

      workingToken = strtok(token[0], "!");
      int workingIndex = atoi(workingToken);

      if(workingIndex < historyCount)
      {
        rerunNPid(token,workingIndex);
      }
      else
      {
        printf("Out of range\n");
      }
    }
    //Rerunning a command shouldnt be added to the history
    else
    {
      addCommandToHistory(token,token_count - 1);
    }



    //cd input
    if(strcmp(token[0],"cd") == 0)
    {
      int ch = chdir(token[1]);
      if(ch != 0 )
      {
        printf("Directory does not exist\n");
      }
    }
    //quit input
    else if(strcmp(token[0],"quit") == 0)
    {
      exit(0);
    }
    //history input
    else if(strcmp(token[0],"history") == 0)
    {
      printf("showing history:\n");
      printHistory();
    }
    //pidhistory input
    else if(strcmp(token[0],"pidhistory") == 0)
    {
      printf("showing pidhistory:\n");
      printPidHistory();
    }
    //for all other commands fork
    else
    {
      pid_t child_pid = fork();

      if(child_pid == 0)
      {
        int ret = execvp(token[0], &token[0]);
        if(ret == -1)
        {
          printf("Command not found\n");
          exit(0);
        }
      }
      else
      {
        updatePidHistory(child_pid);
      }
      int status;
      waitpid(child_pid, &status, 0);

    }



    free( head_ptr );

  }
  return 0;
  // e2520ca2-76f3-11ec-90d6-0242ac120003
}

void addCommandToHistory(char* token[], int numOfArgs)
{

  struct command *temp = (struct command*)malloc(sizeof(struct command));
  for(int i = 0; i < numOfArgs; i++)
  {
    temp->command[i] = token[i];
  }
  temp->numOfArgs = numOfArgs;
  //defualt pid is -1 until updatePidHistory is called for forked commands
  temp->pid = -1;
  //if first  command
  if(head == NULL)
  {
    head = temp;
    temp->next = sentinel;
    historyCount = 1;
  }

  else
  {
    int numOfPreviousCommands = 1;
    struct command *traversingNode = head;
    //traverses queue
    while(traversingNode->next != sentinel)
    {
      traversingNode = traversingNode->next;

    }
    //add temp node to the last command
    traversingNode->next = temp;
    temp->next = sentinel;

    //if queue full
    if(historyCount == MAX_RECORDED_COMMANDS)
    {
      struct command *deletedTemp = head;
      head = head->next;
      //undos historyCount++ at the end of addCommandToHistory
      historyCount--;
      free(deletedTemp);
    }
    historyCount++;
  }
}
//traverses Pid history linked list and prints it to terminal
void printHistory()
{
  struct command *traversingNode = head;
  int index = 0;
  //history is empty
  if(head == NULL)
  {
    return;
  }
  printf("%d)   ",index);
  printCommand(traversingNode);
  while(traversingNode->next != sentinel)
  {
    index++;
    traversingNode = traversingNode->next;
    printf("%d)   ",index);
    printCommand(traversingNode);

  }
}

void printPidHistory()
{
  struct command *traversingNode = head;
  int index = 0;
  //history is empty
  if(head == NULL)
  {
    return;
  }

  printf("%d)   %d\n",index, traversingNode->pid);
  while(traversingNode->next != sentinel)
  {
    index++;
    traversingNode = traversingNode->next;
    //if traversingNode does not have a pid (pid == default value)
    if(traversingNode->pid == -1)
    {
      printf("%d)   no pid\n",index);
    }
    else
    {
      printf("%d)   %d\n",index, traversingNode->pid);
    }

  }
}

void printCommand(struct command *selectedCommand)
{
  //iterrates through all the arguments in the command and prints
  for(int i = 0; i < selectedCommand->numOfArgs; i++)
  {

      printf("%s", selectedCommand->command[i]);
      printf(" ");
  }
  printf("\n");

}
//updates token with requested Command's token
void rerunNPid(char* token[MAX_NUM_ARGUMENTS],int n)
{

  struct command *temp = head;
  //finds requested command through queue
  for(int i = 0; i < n; i++)
  {
    temp = temp->next;
  }
  //updates token with requested commands's token
  for(int i = 0; i < temp->numOfArgs; i++)
  {
    token[i] = temp->command[i];
  }
}

void updatePidHistory(pid_t pid)
{
  struct command *temp = head;
  while(temp->next != sentinel)
  {
    temp = temp->next;
  }
  //updates last added command's pid
  temp->pid = pid;
}
