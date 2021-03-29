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
//
//
// Purpose: Demonstrate the use of semaphore with a producer / consumers problem.
// In this example producer reads characters one by one from a file and writes them sequentially
// in a circular queue. The consumer reads sequencially from the queue and prints them in the
// same order

//Shreya Bhatta 1001736276
//Find number of substrings in a given file using threading.
//To Compile: gcc -o proCon proCon.c -lpthread
//            ./proCon 

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX 5000

#define BufferSize 5
#define NONSHARED 1

pthread_mutex_t mutex;

char *s;
int n,j=0;
int queue[BufferSize];

sem_t chars_full ;
sem_t chars_in_line;    
int queueIndex=0; //counting chars num in buffers

int count=0;
FILE *fp;
char *s;

void * Producer( void * arg ) 
{
  int i=0;
  fp=fopen("message.txt", "r");
  s=(char *)malloc(sizeof(char)*MAX);
  while((n=fgetc(fp)) != EOF)
  {
    s[i++] = (char)n;
    // wait for buffer to be empty if it's fulll
    sem_wait( &chars_full );
    pthread_mutex_lock(&mutex);//lock access to queue
    queue[queueIndex++] = s[j++]; 
    pthread_mutex_unlock(&mutex); //unlock access to queue
    sem_post(&chars_in_line); //notify the consumer we've added something
   
  }  
  s[i] = '\0';
}


void * Consumer( void * arg ) 
{
  while( n != EOF)
  {
    sem_wait( &chars_in_line ); //waits until something is produced
    pthread_mutex_lock(&mutex); //lock access to queue
    printf("%c\n",queue[queueIndex]);
    queue[queueIndex--] = '\0';
    pthread_mutex_unlock(&mutex); //unlock access to queue
    sem_post( &chars_full);

  }

}

int main( int argc, char *argv[] ) 
{
  time_t t;

  srand((unsigned int)time(&t));

  pthread_t producer_tid;  
  pthread_t consumer_tid ; 
  pthread_mutex_init(&mutex,NULL); 

  sem_init( & chars_full, 0, BufferSize );  
  sem_init( & chars_in_line, 0, 0 ); 
 
  pthread_create( & producer_tid, NULL, Producer, NULL );
  pthread_create( & consumer_tid, NULL, Consumer, NULL );
  

   pthread_join( producer_tid, NULL );
   pthread_join( consumer_tid, NULL );


  exit(0);
}