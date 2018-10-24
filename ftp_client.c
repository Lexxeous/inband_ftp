#include "InBand_FTP.h"

const int CLI_BUF_LEN = 512; // global variable to allocate different amounts of chars
const int FTP_CMD_LEN = 3; //
const int NO_CLI_BUF_OV = 5; // prevent overflow when appending filename
const int CTS_OR_ERR_LEN = 4;

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
    bool valid_terminal_in = false; //assume the user terminal input is invalid
    bool print_serv_resp = false;
    bool chk0 = false;
    bool chk1 = false;
    bool chk2 = false;
    bool chk3 = false;

    int buff_len;

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

    char* cts_or_err = (char*)malloc(CTS_OR_ERR_LEN); // "CTS:" or "ERR:"
    memset(cts_or_err, '\0', CTS_OR_ERR_LEN); // clear the file content buffer

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


    // check for "quit" and sift through inputs
    if(!strcmp(buff, "quit"))
    {
      closed = true; // closed the client connection
      sendMessage(&info, "quit");
      printf("\n");
      continue;
    }
    else if (!strcmp(buff, "help"))
    {
      printf("The avaliable FTP commands are: quit, get <filename>, & put <filename>.\n");
      continue;
    }
    else
    {
      // as long as the indexes exist, check each index up to buff[3]
      if ((buff_len - 1) > 0)
        chk0 = ((buff[0] == 'p') || (buff[0] == 'g'));
      if ((buff_len - 1) > 1)
        chk1 = ((buff[1] == 'u') || (buff[1] == 'e'));
      if ((buff_len - 1) > 2)
        chk2 = (buff[2] == 't');
      if ((buff_len - 1) > 3)
        chk3 = (buff[3] == ' ');


      if((buff_len - 1) < 3) //garbage less than 3 characters
      {
        printf("ERR:400 bad request, FTP command too short.\n");
        printf("Type \"help\" for the list of FTP commands.\n");
        continue;
      }
      else if(!strcmp(buff, "put") || !strcmp(buff, "get")) // if explicitly "get" or "put"
      {
        if(!strcmp(buff, "put"))
        {
          printf("ERR:400 bad request, improper command format and filename not included for storage.\n");
          continue;
        }
        else
        {
          printf("ERR:400 bad request, improper command format and filename not included for retrieval.\n");
          continue;
        }
      }
      else if(!strcmp(buff, "put ") || !strcmp(buff, "get ")) // if explicitly "get " or "put "
      {
        if(!strcmp(buff, "put "))
        {
          printf("ERR:400 bad request, filename not included for storage.\n");
          continue;
        }
        else
        {
          printf("ERR:400 bad request, filename not included for retrieval.\n");
          continue;
        }
      }
      else if((strstr(buff, "put ") == NULL) && (strstr(buff, "get ") == NULL))
      {
        printf("ERR:403 forbidden, improper command format.\n");
        continue;
      }
      else if(!(chk0 && chk1 && chk2 && chk3)) // must start explicitly with "get " or "put "
      {
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
      if(!strcmp(ftp_cmd, "put"))
      {
        if((cli_file = fopen(f_name, "r")) == NULL) // if "f_name" does not exist in "client_dir"
        {
          printf("ERR:404 %s does not exist in the local client directory.\n", f_name);
          deallocate_message(ftp_cmd);
          continue;
        }
        else
        {
          print_serv_resp = true;
          memcpy(protocol_msg, "STOR:", 5); // protocol_msg = "STOR:"
          strcpy(user_cmd, protocol_msg); // prepend "STOR:"
          strcat(user_cmd, f_name); // append <filename>
          sendMessage(&info, user_cmd); // send STOR:<filename> to server
        }
      }
      else if (!strcmp(ftp_cmd, "get"))
      {
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
    // if(print_serv_resp)
    printf("%s\n", serv_resp);


    memcpy(cts_or_err, serv_resp, CTS_OR_ERR_LEN);
    printf("%s\n", cts_or_err);
    if(!strcmp(cts_or_err, "CTS:"))
    {
      printf("CLI GOT CTS: FROM SERVER\n");
    }
    else
    {
      printf("CLI GOT ERR: FROM SERVER\n");
    }


    deallocate_message(serv_resp);
    deallocate_message(ftp_cmd);
    deallocate_message(cts_or_err);
  }

  return 0;
}
