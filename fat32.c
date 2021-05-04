//Shreya Bhatta 1001736276
//compilation command: gcc -o fat32 fat32.c
//                     ./fat32 


// The MIT License (MIT)
// 
// Copyright (c) 2020 Trevor Bakker 
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
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>



#define MAX_NUM_ARGUMENTS 3

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

FILE *fp;

struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t unused[8];
    uint16_t ClusterHigh;
    uint8_t unused2[4];
    uint16_t ClusterLow;
    uint32_t size;
};

struct DirectoryEntry dir[16];

int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t   BPB_NumFATS;
int32_t BPB_FATSz32;

//converts from cluster to offset
int LBAToOffset(int32_t sector)
{
  if(sector == 0)
  {
    sector =2;
  }
  return((sector-2)* BPB_BytesPerSec) + (BPB_BytesPerSec* BPB_RsvdSecCnt) +
             (BPB_NumFATS *BPB_FATSz32 * BPB_BytesPerSec);
}

//Handles FAT. Each cluster of file points to next, if no further blocks returns -1;
int16_t NextLB(uint32_t sector)
{
  uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt)+(sector*4);
  int16_t val;
  fseek(fp, FATAddress, SEEK_SET);
  fread(&val,2,1, fp);
  return val;
}

//returns 1 if the strings match, 0 if they don't match;
int compare(char IMG_Name[], char input[])
{
  char expanded_name[12];
  memset( expanded_name, ' ', 12 );
  char *token = strtok( input, "." );
  strncpy( expanded_name, token, strlen( token ) );

  token = strtok( NULL, "." );

  if( token )
  {
    strncpy( (char*)(expanded_name+8), token, strlen(token ) );
  }
  expanded_name[11] = '\0';
  int i;
  for( i = 0; i < 11; i++ )
  {
    expanded_name[i] = toupper( expanded_name[i] );
  }
  if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
  {
    return 1;
  }
  return 0;
}

int main()
{
  int file_closed =0; //file has not been opened
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  int root_dir;
  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }
      
    //if nothing is entered continue with shell prompt
    if(token[0] == NULL)
    {
      continue;
    }

    else if(strcmp(token[0],"open")==0)
    {
      if(file_closed == 0) //if file is not currently open,open the file
      {
        fp = fopen(token[1],"r");
        if(!fp) 
        {
          printf("Error: File system image not found\n");
        }
        else
        {
          file_closed =1;  //the file is now open
          fseek(fp,11, SEEK_SET);
          fread(&BPB_BytesPerSec,2,1,fp);

          fseek(fp,13, SEEK_SET);
          fread(&BPB_SecPerClus,1,1,fp);


          fseek(fp,14, SEEK_SET);
          fread(&BPB_RsvdSecCnt,2,1,fp);


          fseek(fp,16, SEEK_SET);
          fread(&BPB_NumFATS,1,1,fp);


          fseek(fp,36, SEEK_SET);
          fread(&BPB_FATSz32,4,1,fp);

          root_dir = (BPB_NumFATS* BPB_FATSz32*BPB_BytesPerSec) +
                          (BPB_RsvdSecCnt * BPB_BytesPerSec);

          fseek(fp, root_dir, SEEK_SET); //fseek the file starting at root directoty
          fread(&dir[0], sizeof(struct DirectoryEntry),16,fp); //read the file to dir
        }
      }
      else
      {
         printf("File system image already open\n");
      }
    }

    else if(strcmp(token[0],"ls")==0)
    {
      if(file_closed==0)
      {
        printf("File system image must be opened first\n");
      }
      else
      {
        int i;
        for(i=0; i<16; i++)
        {
          char filename[12]; //temp file with null terminator
          strncpy(&filename[0], &dir[i].DIR_Name[0],11);
          filename[11]='\0';
          if(((dir[i].DIR_Attr == 0x01) || (dir[i].DIR_Attr == 0x10) ||
              (dir[i].DIR_Attr == 0x20)) && (dir[i].DIR_Name[0] !=0xe5))
              {
                printf("%s\n",filename);
              }
        }
      }

    }  
    else if(strcmp(token[0],"info")==0)
    {
      if(file_closed==0)
      {
        printf("File system image must be opened first\n");
      }
      else
      {
         printf("BPB_BytesPerSec:  %d   %x\n", BPB_BytesPerSec,BPB_BytesPerSec);
         printf("BPB_SecPerClus:   %d   %x\n", BPB_SecPerClus,BPB_SecPerClus);
         printf("BPB_RsvdSecCnt:   %d   %x\n", BPB_RsvdSecCnt,BPB_RsvdSecCnt);
         printf("BPB_NumFATS:      %d   %x\n", BPB_NumFATS,BPB_NumFATS);
         printf("BPB_FATSz32:      %d   %x\n", BPB_FATSz32,BPB_FATSz32);
      
      }
    }

    else if(strcmp(token[0],"close")==0)
    {
      if(file_closed==0)
      {
        printf("File system image must be opened first\n");
      }
      else
      {
      file_closed =0;
      fclose(fp);
      }
    }

    else if(strcmp(token[0],"cd")==0)
    {
      if(file_closed==0)
      {
        printf("File system image must be opened first\n");
      }
      else
      {
        int i;
        for(i=0; i<16; i++)
        {
          if((compare(dir[i].DIR_Name, token[1]))==1)
          {
            fseek(fp, LBAToOffset(dir[i].ClusterLow),SEEK_SET);
            fread(&dir[0], sizeof(struct DirectoryEntry),16,fp);   
          }
        }   
      }
    }
    else if(strcmp(token[0],"stat")==0)
    {
      int match =0; //if the file is not found
      if(file_closed==0)
      {
        printf("File system image must be opened first\n");
      }
      else
      {
        int i;
        for(i=0; i<16; i++)
        {
          if((compare(dir[i].DIR_Name, token[1]))==1)
          {
            match =1; //the file is found
            if(dir[i].ClusterLow == 0)
            {
              dir[i].ClusterLow == 2; //change the clusterlow of root directory to2
            }
            printf("File Size: %d\n", dir[i].size);
            printf("First Cluster Low: %d\n", dir[i].ClusterLow);
            printf("DIR_ATTR: %d\n", dir[i].DIR_Attr);
            printf("First Cluster High: %d\n", dir[i].ClusterHigh);
          }
        }   
        if(match ==0)
        {
          printf("Error:File not found\n");
        }
      }
    }

   /*  
    else if(strcmp(token[0],"get")==0)
    {
      int match =0; //if the file is not found
      if(file_closed==0)
      {
        printf("File system image must be opened first\n");
      }
      else
      {
        int i;
        for(i=0; i<16; i++)
        {
          if((compare(dir[i].DIR_Name, token[1]))==1)
          {
            match =1; //the file is found
            FILE *ofp = fopen(token[1],"w");
            if(!ofp)
            {
              printf("There was an error opening file\n");
            }
            uint32_t currLB = dir[i].ClusterLow;
            //write each block of file to the new file in directory.
            for(int j= dir[i].size; j<512; j-=512)
            {
              unsigned char buffer[512];
              fseek(fp, LBAToOffset(currLB), SEEK_SET);
              fread(&buffer, 512,1,fp);
              fwrite(&buffer,512,1, ofp);
              currLB = NextLB(currLB);
            }  
          }
        }   
        if(match ==0)
        {
          printf("Error:File not found\n");
        }

      }
    }  
    */
    else if((strcmp(token[0],"read")==0) && (token[1]!=NULL) && (token[2] !=NULL))
    {
      if(file_closed==0)
      {
        printf("File system image must be opened first\n");
      }
      else
      {
        int match =0; //if the file is not found
        int position = atoi(token[2]);
        int numBytes = atoi(token[3]);
        int i;
        for(i=0; i<16; i++)
        {
          unsigned char buffer[numBytes];
          if((compare(dir[i].DIR_Name, token[1]))==1)
          {
            match =1;
            fseek(fp, LBAToOffset(dir[i].ClusterLow)+position,SEEK_SET);
            fread(&buffer, numBytes, 1, fp);
            printf("%s \n",buffer);
          }    
        }
        if(match ==0)
        {
          printf("Error:File not found\n");
        }
      }
    }  
    else
    {
      printf("Command not found");
    }
   
    free( working_root );
  }
  return 0;
}