#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "client_api.h"
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h> 
#define rdtsc __asm __emit 0fh __asm __emit 031h 
#define cpuid __asm __emit 0fh __asm __emit 0a2h

double singlerun(int i){
    unsigned long lo,hi,result,result0,result1;
    double frequency = 3591556000.0;
    double total_api = 5.0;
    char name[8]="file";
    char mark[2]="";
    sprintf(mark, "%d", i);
    strcat(name, mark);
    // set a buf message to be writen in the file.in
    char writebuf[] = "this is a message";
    struct fileStat *statbuf;
    // make a space for the fileStat struct to store the file info
    statbuf = (struct fileStat *) malloc (sizeof(struct fileStat));
    
    __asm__ __volatile__(
            "cpuid\n\t"
            "rdtsc\n\t"
            :"=a" (lo),"=d" (hi)
            :
        );
    result0 = ((unsigned long long)lo)| (((unsigned long long)hi)<<32);
    // set server to be cd.cs.rutgers.edu port number to be: 16892
    setServer("cd.cs.rutgers.edu",16892);
    // open the file.in
    int fd1 = openFile(name);
    // write into the file.in
    int writereturn = writeFile(fd1, writebuf);
    // get file.in stats
    int a = statFile(fd1,statbuf);
    //printf("file.in information: is: size: %d\tcreate time: %s\taccess time: %s\tmodification time: %s\n", statbuf->fsize, statbuf->c_time, statbuf->a_time, statbuf->m_time);
    closeFile(fd1);
    __asm__ __volatile__(
            "cpuid\n\t"
            "rdtsc\n\t"
            :"=a" (lo),"=d" (hi)
            :
            );
    result1 = ((unsigned long long)lo)| (((unsigned long long)hi)<<32);
    result = result1 - result0;
    double total_cycle = (double) result;
    double throughput = (total_api * frequency )/ total_cycle;
    free(statbuf);
    printf("%f\n", throughput);
    return throughput;
}

int main()
{
  double throughput[60];
  printf("start test\n");
  singlerun(1);
  // allocate child process id
  pid_t child_pids[60];
  int p = 60;
  int i;
  // a temporary process id
  pid_t pid;
  for (i = 0; i < p; ++i)
  {
    pid = child_pids[i] = fork();
    if(pid == 0)  // child process
    {
      //printf("process number: %d\n", getpid());
      throughput[i] = singlerun(i); // create a new process and pass it the input parameters
      printf("%f\n", throughput[i]);
      exit(0);
    }

  }
  if (pid != 0)  // parent process
  {
    for (i = 0; i < p; ++i)
    {
      int status;
      waitpid(child_pids[i], &status, 0);
    }
  }

	return 0;
}