#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
int main( )
{
  pid_t pid = fork( );
  printf("%d",strlen("/bin/"));
  if( pid == 0 )
  {
    // Notice you can add as many NULLs on the end as you want
    int ret = execl( "/bin/ls", "ls", "-a", "-l", "-t", NULL, NULL, NULL, NULL );

    if( ret == -1 )
    {
      perror("execl failed: ");
    }
  }
  else
  {
    int status;
    wait( & status );
  }

  return 0;
}
