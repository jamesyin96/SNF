#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "client_api.h"


int main()
{
	// set server to be cd.cs.rutgers.edu port number to be: 16892
	setServer("cd.cs.rutgers.edu",16892);
	// open the file.in
	int fd1 = openFile("file.in");
	// set a buf message to be writen in the file.in
	char writebuf[] = "this is a message";
	// write into the file.in
	int writereturn = writeFile(fd1, writebuf);
	// make a space for the fileStat struct to store the file info
	struct fileStat *statbuf;
  	statbuf = (struct fileStat *) malloc (sizeof(struct fileStat));
  	// get file.in stats
  	int a = statFile(fd1,statbuf);
  	printf("file.in information: is: size: %d\tcreate time: %s\taccess time: %s\tmodification time: %s\n", statbuf->fsize, statbuf->c_time, statbuf->a_time, statbuf->m_time);
  	
  	// open reverse.in
  	int fd2 = openFile("reverse.in");

  	char readbuf[1024]="";
  	// read the content of file.in into readbuf
  	int readreturn = readFile(fd1, readbuf);

  	char writebuf2[1024]="";
  	int filesize = statbuf->fsize;
  	// reverse the content of the file
  	int i;
    for (i = 0; i < filesize; i++)
  	{
  		writebuf2[i] = readbuf[filesize - i - 1];
  		
  	}

  	// write the reversed message into reverse.in
  	writeFile(fd2, writebuf2);

  	struct fileStat *statbuf2;
  	statbuf2 = (struct fileStat *) malloc (sizeof(struct fileStat));
  	// get file.in stats
  	int b = statFile(fd2,statbuf2);
  	printf("file.in information: is: size: %d\tcreate time: %s\taccess time: %s\tmodification time: %s\n", statbuf2->fsize, statbuf2->c_time, statbuf2->a_time, statbuf2->m_time);

  	free(statbuf2);
  	free(statbuf);
	return 0;
}