#include "InBand_FTP.h"

const int CLI_BUF_LEN = 512; // global variable to allocate different amounts of chars
const int FTP_CMD_LEN = 3; //

int main(int argc, char* argv[])
{
  // error check the terminal command
  if(argc < 3)
  {
    printf("Give server name and sever port as command line arguments.\n");
    return 1;
  }

  char* server_name = argv[1];
  int server_port = atoi(argv[2]); // ASCII to integer

  ConnectionInfo info;
  if(connect_to_server(server_name, server_port, &info))
  {
    printf("Failed to connect to server.\n");
    return 1;
  }

  printf("Type \"help\" for the list of FTP commands.\n");

  bool closed = false;

  while(!closed)
  {
    char buff[CLI_BUF_LEN]; // user terminal input
    char protocol_msg[CLI_BUF_LEN]; // "STOR:", "RETV:", "CTS:", "CONT:", "ERR:"
    char user_cmd[CLI_BUF_LEN]; // PMSG:<filename>
    char ftp_cmd[FTP_CMD_LEN]; // "get" & "put"


    // read terminal input
    printf("\nEnter an FTP command:\n");
    fgets(buff, CLI_BUF_LEN, stdin);


    if(!strcmp(buff, "\n")) // if user inputs ENTER
    {
      continue; // go to next loop iteration
    }
    else // if the user inputs any string
    {
      int len = strlen(buff); // length of buff including '\n'
      buff[len - 1] = 0; // remove trailing '\n' from buff
    }


    if(!strcmp(buff, "quit"))
      closed = true; // closed the client connection
    

    memcpy(ftp_cmd, buff, 3); // copy the first 3 characters of buff into "ftp_cmd"


    if(!strcmp(ftp_cmd, "put")) // put filename.txt
    {
      strcpy(protocol_msg, "STOR:"); // prepend PMSG:
      strcpy(user_cmd, "STOR:");
      memcpy(user_cmd + 5, buff + 4, CLI_BUF_LEN-5); // append <filename>
      sendMessage(&info, user_cmd); // send PMSG:<filename> to server
    }
    else if (!strcmp(ftp_cmd, "get"))
    {
      strcpy(protocol_msg, "RETV:"); // prepend PMSG:
      strcpy(user_cmd, "RETV:");
      memcpy(user_cmd + 5, buff + 4, CLI_BUF_LEN-5); // append <filename>
      sendMessage(&info, user_cmd); // send PMSG:<filename> to server
    }
    else
    {
      sendMessage(&info, buff); // send terminal input without trailing '\n'
    }

    // receive the server response
    char* serv_resp = receiveMessage(&info);
    if(serv_resp == NULL)
    {
      printf("Failed to receive message.\n");
      return 1;
    }
    printf("%s\n", serv_resp); // print the server response message
    deallocate_message(serv_resp); // free (deallocate) message memory
  }

  return 0;
}
