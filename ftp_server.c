#include "InBand_FTP.h"

int main(int argc, char* argv[])
{
  int port = atoi(argv[1]); // starting port number
  int retries = 100; // number of retries before failure

  while(retries --) // try to start server 100 times
  {
    if(!run_server(port))
    {
      while(1)
      {
        sleep(10); // number of seconds of wait time before retry
      }
    }
    else
    {
      port ++; // increment port number if failure on previous port
    }
  }
  printf("Failed to start server.\n");
  return 1;
}
