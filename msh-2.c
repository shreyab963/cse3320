/*
name:Shreya Bhatta
ID: 1001736276
*/

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

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports 10 arguments in addition to the command

#define MAX_HIST 15             // Mav shell only supports 15 user commands
                                // for history and pids

char* history[MAX_HIST];        //stores commands for history
int pids[MAX_HIST];             //stores commands for listpid

int n=0;                        //To count number of processes for listpid

//accepts PIDS of each process and stores in an array
void store_pid(int _pid)
{
  if(n < MAX_HIST)
  {
    pids[n] = _pid;
  }
  else
  {
    //if processes count is more than 15, stores the most recent 15 PIDS
    for(int i=0; i< MAX_HIST; i++) 
    {
      pids[i-1] = pids[i];
    }
    pids[MAX_HIST-1]= _pid;
  }
}

//prints the PIDS stored in pids[MAX_HIST] array when the user imputs listpid
void print_pids()
{
  for(int i=0; i<n; i++)
   {
     printf("%d: %d\n",i, pids[i]);
   }
}

//forks a child process and executes commands using exec
//accepts input command and count of arguments in a command line
void run_cmd(char** cmd_string, int count)
{
  pid_t pid = fork();
  if(pid < 0)
  {
     printf("\nCould not fork");
     return;
  }
  if( pid == 0 )
  {
    int i=0;

    //to make a copy of cmd_string since strncpy will make changes to cmd_string
    char *arguments[count];

    //adds the PID number of the process to store_pid and increments PID count
    store_pid(getpid()); 
    n++;

    for(i=0; i<count-1; i++)
    {
      arguments[i] = ( char * ) malloc( strlen( cmd_string[i] ) );
      strncpy( arguments[i], cmd_string[i], strlen( cmd_string[i]  ) );
    }
    arguments[i] = NULL; //sets the last string to NULL
    
    int ret = execvp( arguments[0], &arguments[0] );  
    if( ret == -1 )
    {  
      printf("\n%s : Command not found \n",arguments[0]); 
    }
  }
  else 
  {
    //adds the PID number of the process to store_pid and increments PID count
  store_pid(getpid()); 
  n++;

    //waits for child to terminate
    int status;
    wait( & status );
  }
}


//accepts each executed command and stores in an array
void store_hist(char* cmd_hist, int n)
{
  if(n< MAX_HIST)
  {
    history[n-1] = strdup(cmd_hist);
  }
  else
  {
    //if processes count is more than 15, stores the most recent 15 PIDS
    free(history[0]);
    for(int i=0; i< MAX_HIST; i++)
    {
      history[i-1] = history[i];
    }
    history[MAX_HIST-1]= strdup(cmd_hist);
  }

}

//prints the commmands stored in history[MAX_HIST] array when the user inputs history
//accepts number of commands entered so far
void print_history(int n)
{
   for(int i=0; i<n; i++)
   {
     printf("%d: %s",i, history[i]);
   }

}
int main()
{
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  int i=0; //keep track of number of command entered by user
  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    //if the command is exit or quit, break out of the loop
    if(strcmp(cmd_str, "exit\n")==0 || strcmp(cmd_str, "quit\n")==0){ 
      return 0;
    }
    
    if((cmd_str[0] == '\n') || (cmd_str[0] == ' ') + (cmd_str[0] == '\t'))
    {
      continue;
    }

   
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];
    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;
      
    // Tokenize the input strings with whitespace used as the delimiter
    while ( ( (argument_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
    
    i++;//tells us the number of commands executed for history
    store_hist(cmd_str,i);//adds command to history

    //if the user inputs cd, change the directory
    if(strcmp(token[0], "cd")==0)
    {
      chdir(token[1]);
      continue;
    }
    
    //if the user inputs history, call print_direcroty function to print 
    //last 15 commands
    if(strcmp(token[0], "history")==0  && token[1] == NULL)
    {
      print_history(i);
      continue;
    }
     
    //if the user inputs listpid, call print_pids function to print pids of
    //last 15 processes
    if(strcmp(token[0], "listpid")==0 && token[1] == NULL)
    {
      print_pids();
      continue;
    }
  
    //create a copy of 1st token in command line to avoid making changes to token[0]
    char *temp_token  = strdup( token[0] );  
    char c = temp_token[0]; //stores the 1st character of token[0];

    if((c== '!') && token[1] == NULL) //if the user types !n, get inside loop
    {
      //store the nth term in c
      c = temp_token[1];
      int x = c - '0'; //convert char to int using '0'
      
      if (x <= i) //if the nth command exits re-run it
      {
        //store history of nth term in hist_temp 
        char *hist_temp[1];
        hist_temp[0]  = strdup( history[x] ); 
        strtok(hist_temp[0], "\n");//remove the end line from history[x]
        hist_temp[1] = NULL;
        run_cmd(hist_temp, 2);//re-run the nth command
        continue;
      }
      else //if the nth command doesn't exist, print the following
      {
        printf("Command not in history\n");
      }
      continue;
    }
    else
    {
      //execute command by forking a child
      run_cmd(token, token_count);
    }

    free( working_root );
  }
  return 0;
}