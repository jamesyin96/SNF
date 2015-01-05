struct fileStat{
   int fd;	//file descriptor
   int fsize;	//file size
   char c_time[32]; 	//file creation time
   char a_time[32];	//file last access time
   char m_time[32];	//file last modification time
};

char* host2IpAddr(char *anIpName);
void setServer(char*serverIP,int port);
int openFile(char* name);
int closeFile(int fd);
int readFile(int fd, void* buf);
int writeFile(int fd, void* buf);
int statFile(int fd, struct fileStat *buf);