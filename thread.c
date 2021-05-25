
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

//Shreya Bhatta 1001736276
//Find number of substrings in a given file using threading.
//To Compile: gcc -o thread thread.c -lpthread
//            ./thread hamlet.txt
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX 5000000
#define NumThread 4

pthread_mutex_t mutex;

int total=0;
int num_thread =4;
int n1,n2,n; 
char *s1,*s2;


FILE *fp;


int readf(char* filename)
{
    if((fp=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can’t open %s!\n", filename);
        return 0;
    }
    
    s1=(char *)malloc(sizeof(char)*MAX);
    
    if (s1==NULL)
    {
        printf ("ERROR: Out of memory!\n") ;
        return -1;
    }
    
    s2=(char *)malloc(sizeof(char)*MAX);
    
    if (s1==NULL)
    {
        printf ("ERROR: Out of memory\n") ;
        return -1;
    }
    
    /*read s1 s2 from the file*/
    
    s1=fgets(s1, MAX, fp);
    s2=fgets(s2, MAX, fp);
    n1=strlen(s1); /*length of s1*/
    n2=strlen(s2)-1; /*length of s2*/
    if( s1==NULL || s2==NULL || n1 < n2 ) /*when error exit*/
    {
        return -1;
    }

}

void *num_substring ( void * id)
{
    int tempTotal=0; //finds total strings in current step
    int i,j,k,start,end;
    int count ;
    long tid = (long)id;

    //split the work betwwen processors with start and end
    start = tid * (n1/NumThread);
    end =   (tid +1)* (n1/NumThread);
    for(i=start; i<end; i++)
    {
        count =0;
        for(j = i ,k = 0; k < n2; j++,k++)
        { /*search for the next string of size of n2*/
        
            if (*(s1+j)!=*(s2+k))
            {
                break ;
            }
            else
            {
                count++;
            }
            if (count==n2) 
                total++; /*find a substring in this step*/        
         }
    }
    pthread_mutex_lock(&mutex);  
    total += tempTotal; //add local total from the step to the final global variable to total
    pthread_mutex_unlock(&mutex);
    
    return NULL ;
}
    
int main(int argc, char *argv[])
{
    int count ;
    
    pthread_t tid[NumThread];
    pthread_mutex_init(&mutex,NULL);

    int i;
    long j;

    struct timeval start, end;
    float mtime; 
    int secs, usecs; 

    gettimeofday(&start, NULL);

    if( argc < 2 )
    {
      printf("Error: You must pass in the datafile as a commandline parameter\n");
    }
    
    readf ( argv[1] ) ;

    for(j=0; j<NumThread; j++)
    {
          pthread_create(&tid[j], NULL, num_substring, (void *)j );

    }

    for(i=0; i<NumThread; i++)
    {
          pthread_join(tid[i], NULL); //wait for prior threads to finish
    }

    gettimeofday(&end, NULL);

    secs  = end.tv_sec  - start.tv_sec;
    usecs = end.tv_usec - start.tv_usec;
    mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;

    printf ("The number of substrings is : %d\n" , total) ;
    printf ("Elapsed time is : %f milliseconds\n", mtime );
    return 0 ; 
}
