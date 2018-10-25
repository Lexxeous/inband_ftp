#include "InBand_FTP.h"

const int CLI_FILE_LEN = 80000;
const int CLI_BUF_LEN = 512; // global variable to allocate different amounts of chars
const int NO_CLI_BUF_OV = 5; // prevent overflow when appending filename
const int P_MSG_LEN = 5;
const int CTS_OR_ERR_LEN = 4;
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
    bool valid_terminal_in = false; //assume the user terminal input is invalid
    bool print_serv_resp = false;
    bool chk0 = false;
    bool chk1 = false;
    bool chk2 = false;
    bool chk3 = false;

    int buff_len;

    char buff[CLI_BUF_LEN]; // user terminal input
    memset(buff, '\0', CLI_BUF_LEN); // clear the terminal input buffer

    char protocol_msg[CLI_BUF_LEN]; // "STOR:", "RETV:", "CONT:"
    memset(protocol_msg, '\0', CLI_BUF_LEN); // clear the protocol message buffer

    char cont_cmd[CLI_FILE_LEN + CLI_BUF_LEN]; // "CONT:<cont_byte_len>:<f_content>"
    memset(cont_cmd, '\0', CLI_FILE_LEN + CLI_BUF_LEN); // clear the content command buffer

    char user_cmd[CLI_BUF_LEN]; // PMSG:<filename>
    memset(user_cmd, '\0', CLI_BUF_LEN); // clear the user command buffer

    char f_name[CLI_BUF_LEN]; // <filename>
    memset(f_name, '\0', CLI_BUF_LEN); // clear the file name buffer

    char f_content[CLI_FILE_LEN]; // "<f_content>"
    memset(f_content, '\0', CLI_FILE_LEN); // clear the file content buffer

    char* serv_resp = (char*)malloc(CLI_BUF_LEN); // "CTS:<filename>" or "ERR:409, conflict..."
    memset(serv_resp, '\0', CLI_BUF_LEN); // clear the server response buffer

    char* cont_byte_len = (char*)malloc(CLI_BUF_LEN); // how long the file is in bytes ; 1 byte per char
    memset(cont_byte_len, '\0', CLI_BUF_LEN); // clear the content message buffer

    char* ftp_cmd = (char*)malloc(FTP_CMD_LEN); // ftp command buffer ; "get" or "put"
    memset(ftp_cmd, '\0', FTP_CMD_LEN); // clear the ftp command buffer

    char* cts_or_err = (char*)malloc(CTS_OR_ERR_LEN); // "CTS:" or "ERR:"
    memset(cts_or_err, '\0', CTS_OR_ERR_LEN); // clear the "cts_or_err" buffer

    FILE* cli_file; // pointer to client-side file


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
          memcpy(protocol_msg, "STOR:", P_MSG_LEN); // protocol_msg = "STOR:"
          strcpy(user_cmd, protocol_msg); // prepend "STOR:"
          strcat(user_cmd, f_name); // append <filename>
          sendMessage(&info, user_cmd); // send STOR:<filename> to server
        }
      }
      else if (!strcmp(ftp_cmd, "get"))
      {
        memcpy(protocol_msg, "RETV:", P_MSG_LEN); // protocol_msg = "RETV:"
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
    serv_resp = receiveMessage(&info); // "CTS:<filename>" or "ERR:409 conflict, file already exists in remote directory"
    if(serv_resp == NULL)
    {
      printf("Failed to receive message.\n");
      return 1;
    }
    if(print_serv_resp)
      printf("%s\n", serv_resp);


    memcpy(cts_or_err, serv_resp, CTS_OR_ERR_LEN);
    if(!strcmp(cts_or_err, "CTS:"))
    {
      if(is_empty_file(f_name)) // file has no content
      {
        printf("ERR:000 empty file.\n");
        sendMessage(&info, "ERR:000 empty file.");
      }
      else // file has content
      {
        // get the "f_content" from "f_name" in "client_dir" one byte at a time
        char ch = fgetc(cli_file);
        int ch_cnt = 0;
        while (ch != EOF)
        {
          ch_cnt += 1;
          append_char(f_content, ch);
          ch = fgetc(cli_file);
        }
        printf("\n");

        // form and send the "CONT:<cont_byte_len>:<f_content>" message
        itoa(ch_cnt, cont_byte_len, 10);
        memcpy(protocol_msg, "CONT:", P_MSG_LEN);
        memcpy(cont_cmd, protocol_msg, P_MSG_LEN);
        strcat(cont_cmd, cont_byte_len);
        append_char(cont_cmd, ':');
        strcat(cont_cmd, f_content);
        sendMessage(&info, cont_cmd);
      }
    }
    else if (!strcmp(cts_or_err, "ERR:"))
    {
      printf("%s\n", serv_resp);
    }

    fclose(cli_file); // close file on client-side

    // free all allocated variables
    deallocate_message(serv_resp);
    deallocate_message(cont_byte_len);
    deallocate_message(ftp_cmd);
    deallocate_message(cts_or_err);
  }

  return 0;
}
