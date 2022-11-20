/*
  pipe-sync.c: Use Pipe for Process Synchronization
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// Question: Update this program to synchronize execution of the parent and
// child processes using pipes, so that we are guaranteed the following
// sequence of print messages:
// Child line 1
// Parent line 1
// Child line 2
// Parent line 2


int main()
{
  char *s, buf[1024];
  int ret, stat;

  s  = "Use Pipe for Process Synchronization\n";

  /* create pipe */
  int p1[2],p2[2],p3[2];  //file descriptors

  pipe(p1);
  pipe(p2);
  pipe(p3);
  ret = fork();
  if (ret == 0) {

    /* child process. */
    printf("Child line 1\n");
    write(p1[1],s,strlen(s));
    read(p2[0],buf,strlen(s));
    printf("Child line 2\n");
    write(p3[1],s,strlen(s));
  } 
  else {

    /* parent process */
    
    read(p1[0],buf,strlen(s));
    printf("Parent line 1\n");
    write(p2[1], s, strlen(s));
    read(p3[0],buf,strlen(s));
    printf("Parent line 2\n");
    wait(&stat);
  }
}
