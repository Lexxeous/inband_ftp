#include "InBand_FTP.h"

const int CLI_BUF_LEN = 512; // global variable to allocate different amounts of chars
const int FTP_CMD_LEN = 3; //
const int NO_CLI_BUF_OV = 5; // prevent overflow when appending filename

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
  int buff_len;

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
    bool valid_terminal_in = false; //assume the user terminal input is invalid

    char buff[CLI_BUF_LEN]; // user terminal input
    memset(buff, '\0', CLI_BUF_LEN); // clear the terminal input buffer

    char protocol_msg[CLI_BUF_LEN]; // "STOR:", "RETV:", "CTS:", "CONT:", "ERR:"
    memset(protocol_msg, '\0', CLI_BUF_LEN); // clear the protocol message buffer

    char user_cmd[CLI_BUF_LEN]; // PMSG:<filename>
    memset(user_cmd, '\0', CLI_BUF_LEN); // clear the user command buffer

    char f_name[CLI_BUF_LEN]; // <filename>
    memset(f_name, '\0', CLI_BUF_LEN); // clear the file name buffer

    char* ftp_cmd = (char*)malloc(FTP_CMD_LEN); // ftp command buffer ; "get" or "put"
    memset(ftp_cmd, '\0', FTP_CMD_LEN); // clear the file content buffer

    FILE* cli_file;


    // read terminal input
    printf("\nEnter an FTP command:\n");
    fgets(buff, CLI_BUF_LEN, stdin);


    // check for "\n"
    if(!strcmp(buff, "\n")) // if user hits ENTER
    {
      valid_terminal_in = true; // "\n" is valid user terminal input
      continue; // go to next loop iteration
    }
    else // if the user inputs any string
    {
      buff_len = strlen(buff); // length of buff including '\n'
      buff[buff_len - 1] = '\0'; // remove trailing '\n' from buff
    }


    // check for "quit" and isolate inputs
    if(!strcmp(buff, "quit"))
    {
      valid_terminal_in = true; // "quit" is valid user terminal input
      closed = true; // closed the client connection
    }
    else if (!strcmp(buff, "help"))
    {
      valid_terminal_in = true; // "help" is a valid user terminal input
    }
    else
    {
      if(!strcmp(buff, "put") || !strcmp(buff, "get")) // if explicitly "get" or "put"
      {
        if(!strcmp(buff, "put"))
        {
          valid_terminal_in = false;
          printf("ERR:400 bad request, improper command format and filename not included for storage.\n");
          continue;
        }
        else
        {
          valid_terminal_in = false;
          printf("ERR:400 bad request, improper command format and filename not included for retrieval.\n");
          continue;
        }
      }
      else if(!strcmp(buff, "put ") || !strcmp(buff, "get ")) // if explicitly "get " or "put "
      {
        if(!strcmp(buff, "put "))
        {
          valid_terminal_in = false;
          printf("ERR:400 bad request, filename not included for storage.\n");
          continue;
        }
        else
        {
          valid_terminal_in = false;
          printf("ERR:400 bad request, filename not included for retrieval.\n");
          continue;
        }
      }
      else if(buff_len >= 4 && buff[3] != ' ') // no space provided after "get" or "put"
      {
        valid_terminal_in = false;
        printf("ERR:403 forbidden, improper command format.\n");
        continue;
      }
      else // must be "get _" or "put _" ; filename that is provided starts at buff[4]
      {
        valid_terminal_in = true;
        memcpy(ftp_cmd, buff, FTP_CMD_LEN); // copy the first 3 characters of buff into "ftp_cmd" ; "get" or "put"
        printf("FTP Command: %s\n", ftp_cmd);
        memcpy(f_name, buff + 4, CLI_BUF_LEN - NO_CLI_BUF_OV); // <filename>
        printf("Filename: %s\n", f_name);
      }
    }


    // compare the contents of ftp_cmd only if valid terminal input
    if(valid_terminal_in)
    {
      printf("Valid terminal input...\n");

      if(!strcmp(ftp_cmd, "put"))
      {
        printf("CLIENT WANTS TO PUT\n");
        memcpy(protocol_msg, "STOR:", 5); // protocol_msg = "STOR:"
        strcpy(user_cmd, protocol_msg); // prepend "STOR:"
        strcat(user_cmd, f_name); // append <filename>
        sendMessage(&info, user_cmd); // send STOR:<filename> to server
      }
      else if (!strcmp(ftp_cmd, "get"))
      {
        printf("CLIENT WANTS TO PUT\n");
        memcpy(protocol_msg, "RETV:", 5); // protocol_msg = "RETV:"
        strcpy(user_cmd, protocol_msg); // prepend "RETV:"
        strcat(user_cmd, f_name); // append <filename>
        sendMessage(&info, user_cmd); // send RETV:<filename> to server
      }
      else
      {
        sendMessage(&info, buff); // send terminal input without trailing '\n'
      }
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
    deallocate_message(ftp_cmd);
  }

  return 0;
}
