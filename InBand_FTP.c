#include "InBand_FTP.h"

const int SERV_FILE_SIZE = 80000;
const int SERV_BUF_SIZE = 512; // global variable to allocate different amounts of chars
const int ACC_CONN = 10; // global variable for amount of acceptable connections
const int P_MSG_SIZE = 5;
const int CTS_OR_ERR_SIZE = 4;

void append_char(char* s, char c)
{
  int len = strlen(s);
  s[len] = c;
  s[len+1] = '\0';
}

//------------------------------------------------------------------------------------------

char* itoa(int value, char* result, int base)
{
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do
	{
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr)
	{
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

//------------------------------------------------------------------------------------------

void write_to_new_file(const char* filepath, const char* data)
{
	FILE* fd = fopen(filepath, "w");
	if (fd != NULL)
	{
    fputs(data, fd);
    fclose(fd);
	}
}

//------------------------------------------------------------------------------------------

char* extract_byte_len(char* s)
{
	int start_i = 5;
	int width = 10;
	int max_i = start_i + width;
	char* buf = (char*)malloc(width);
  memset(buf, '\0', width);
	for (int i = start_i; i <= max_i; i++)
	{
		if(s[i] == ':')
			break;
		append_char(buf, s[i]);
	}
	deallocate_message(buf);
	return buf;
}

//------------------------------------------------------------------------------------------

int int_width(int x)
{
	return floor(log10(x)) + 1;
}

//------------------------------------------------------------------------------------------

bool is_empty_file(const char* filepath)
{
	int size;
  FILE* fd = fopen(filepath, "r");
  if (fd != NULL)
  {
    fseek (fd, 0, SEEK_END);
    size = ftell(fd);
    fclose(fd);

    if (0 == size)
    {
      return true;
    }
  }
  fclose(fd);
  return false;
}

//------------------------------------------------------------------------------------------

int sendMessage(struct ConnectionInfo* con, char* msg)
{
	if(con == NULL || msg == NULL) // dont allow empty structs or messages
	{
		return 1;
	}

	send(con->socket, msg, strlen(msg), 0); // send the message

	return 0; // return successfully
}

//------------------------------------------------------------------------------------------

char* receiveMessage(struct ConnectionInfo* con)
{
	char* con_buf = (char*)malloc(SERV_BUF_SIZE); // allocate message memory
	memset(con_buf, '\0', SERV_BUF_SIZE); // clear the "con_buf" buffer

	if(con == NULL) // dont allow empty struct
	{
		return "ERR:999 connection info is empty.\n";
	}

	recv(con->socket, con_buf, SERV_BUF_SIZE, 0); // receive the message

	return con_buf; // return what was sent
}

//------------------------------------------------------------------------------------------

void deallocate_message(char* mem)
{
	free(mem); // free (deallocate) message memory
}

//------------------------------------------------------------------------------------------

int connect_to_server(char* who, int port, struct ConnectionInfo* con)
{
	int sockfd;
	struct sockaddr_in server_addr;
	struct hostent* hent;

	// error check the port number
	if(port < 10001)
	{
		printf("Port ≥ 10001 required.\n");
		return 1;
	}

	if((hent=gethostbyname(who)) == NULL)
	{
		printf("Invalid host name.\n");
		return 1;
	}

	// create the client socket.
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // create socket with IPv4 and stream configuration
	{
		printf("Socket error.\n");
		return 1;
	}

	memset((void*) &server_addr, 0, sizeof(server_addr)); // clear the server address structure

	// set up the server address structure.
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr = *((struct in_addr*)hent->h_addr);
	server_addr.sin_port = htons(port);

	if(connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
	{
		printf("Connect error.\n");
		return 1;
	}

	con->socket = sockfd;

	return 0; // return successfully
}

//------------------------------------------------------------------------------------------

int run_server(int port)
{
	printf("Attempting to run server on port %d...\n", port);

	int sockfd, newsockfd; // set up sockets ; fd = file descriptor
	int f_byte_len; //length of file in bytes
	unsigned int client_length;
	struct sockaddr_in server_addr, cli_addr; // structures for client and server addresses.
	struct ConnectionInfo con;


	// create the server socket.
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // fail if return -1
	{
		printf("Socket error.\n");
		return 1;
	}


	// set up the server address structure
	memset((void*) &server_addr, 0, sizeof(server_addr)); // clear the server address structure
	server_addr.sin_family = AF_INET; // use IPv4
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // accept any connections
	server_addr.sin_port = htons(port); // use network byte order (big-endian)


	// bind the socket to the server address and port
	if(bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
	{
		printf("Bind error.\n");
		return 1;
	}
	printf("Successfully bound server on port %d.\n", port);


	// listen on the socket, queue ACC_CONN incoming connections
	listen(sockfd, ACC_CONN);


	while(1) // loop forever
	{
		client_length = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &client_length);
		if(newsockfd < 0)
		{
			printf("Accept error.\n");
			return 1;
		}
		printf("Successfully connected client to server on port %d...\n\n", port);


		bool closed = false;


		while(!closed) // loop while the client is still conencted
		{
			int cont_byte_size; // CONT:"<cont_byte_size>":<f_content>
			int cont_byte_width; // the width of "cont_byte_size"

			char* cont_cmd = (char*)malloc(SERV_FILE_SIZE + SERV_BUF_SIZE); // "CONT:<cont_byte_len>:<f_content>"
    	memset(cont_cmd, '\0', SERV_FILE_SIZE + SERV_BUF_SIZE); // clear the content message buffer

    	char* cont_byte_len = (char*)malloc(SERV_BUF_SIZE); // CONT:"<cont_byte_len>":<f_content> ; 1 byte per char
    	memset(cont_byte_len, '\0', SERV_BUF_SIZE); // clear the content message buffer

			char* user_cmd = (char*)malloc(SERV_BUF_SIZE); // "user_cmd" buffer ; "PMSG:filename.ext"
			memset(user_cmd, '\0', SERV_BUF_SIZE); // clear the user_cmd buffer

			char* con_buf = (char*)malloc(SERV_BUF_SIZE); // "ConnectionInfo" buffer ;
			memset(con_buf, '\0', SERV_BUF_SIZE); // clear the buffer

			char* protocol_msg = (char*)malloc(SERV_BUF_SIZE); // protocol message buffer ; "PMSG:"
			memset(protocol_msg, '\0', SERV_BUF_SIZE); // clear the protocol message buffer

			char* protocol_resp = (char*)malloc(SERV_BUF_SIZE); // protocol response buffer ; "CTS:filename.ext" ; "CONT:f_byte_len:content"
			memset(protocol_resp, '\0', SERV_BUF_SIZE); // clear the protocol response buffer

			char* f_name = (char*)malloc(SERV_BUF_SIZE); // file name buffer ; filename.ext
			memset(f_name, '\0', SERV_BUF_SIZE); // clear the "f_name" buffer

			char* f_content = (char*)malloc(SERV_FILE_SIZE); // file content buffer
			memset(f_content, '\0', SERV_FILE_SIZE); // clear the file content buffer

			char* cts_or_err = (char*)malloc(CTS_OR_ERR_SIZE); // file name buffer ; filename.ext
			memset(cts_or_err, '\0', CTS_OR_ERR_SIZE); // clear the "f_name" buffer

			FILE* serv_file;

			// set up ConnectionInfo structure for the server
			con.socket = newsockfd;
			con.buf = con_buf;
			con.buf_length = SERV_BUF_SIZE;


			// "quit", "STOR:<filename>", "RETV:<filename>"
			user_cmd = receiveMessage(&con);
			printf("user_cmd: %s\n", user_cmd);


			// separate user commands
			if(strcmp(user_cmd, "quit")) // skip if user sent "quit"
			{
				memcpy(protocol_msg, user_cmd, P_MSG_SIZE); // protocol_msg = "PMSG:"
				printf("protocol_msg: %s\n", protocol_msg); // PMSG:
				memcpy(f_name, user_cmd + 5, SERV_BUF_SIZE - 5); // f_name = "<filename>"
				printf("f_name: %s\n\n", f_name); // <filename>
			}


			if(!strcmp(user_cmd, "quit")) // if client sends "quit" command
			{
				printf("Closing client connection...\n");
				closed = true;
				close(newsockfd);
			}
			else if(!strcmp(protocol_msg, "RETV:")) // if client sends "get <filename>" command
			{
				// // if "f_name" exists in "server_dir"
				// memcpy(protocol_resp, "CONT:", 5);

				// // get the file size in bytes (f_byte_len)
				// strcat(protocol_resp, (char*)f_byte_len);
				// strcat(protocol_resp, ":");

				// // get content of file in ASCII (f_content)
				// strcat(protocol_resp, f_content);
				// printf("%s\n", protocol_resp);
				// sendMessage(&con, protocol_resp);
				// // else

				// sendMessage(&con, "ERR:404 file does not exist in remote directory.");
			}
			else if(!strcmp(protocol_msg, "STOR:")) // if client sends "put <filename>" command
			{
				if((serv_file = fopen(f_name, "r")) == NULL) // if "f_name" does not exist in "server_dir"
				{
					memcpy(cts_or_err, "CTS:", CTS_OR_ERR_SIZE);
					memcpy(protocol_resp, cts_or_err, CTS_OR_ERR_SIZE);
					strcat(protocol_resp, f_name);
					sendMessage(&con, protocol_resp);
					fclose(serv_file);
				}
				else // "f_name" already exists in "server_dir"
				{
					sendMessage(&con, "ERR:409 conflict, file already exists in remote directory");
				}
			}
			else
			{
				sendMessage(&con, "Invalid command. Type \"help\" for the list of FTP commands.\n");
			}


			if(!strcmp(cts_or_err, "CTS:"))
			{
				cont_cmd = receiveMessage(&con); // "CONT:<cont_byte_len>:<f_content>" or "ERR:000 empty file."

				if(strstr(cont_cmd, "CONT:"))
				{
					cont_byte_len = extract_byte_len(cont_cmd); // get the "cont_byte_len" string
					cont_byte_size = atoi(cont_byte_len); // convert "cont_byte_len" string to "cont_byte_size" integer
					cont_byte_width = int_width(cont_byte_size); // width of the "cont_byte_size"

					int shift = P_MSG_SIZE + cont_byte_width + 1; // calculate shift to <f_content>
					memcpy(f_content, cont_cmd + shift, SERV_FILE_SIZE - shift); // copy <f_content> to "f_content"
					write_to_new_file(f_name, f_content); // create new file in "server_dir"
				}
				else if (strstr(cont_cmd, "ERR:"))
				{
					printf("...failed STOR.\n\n");
				}
			}


			// free all allocated variables
			deallocate_message(cont_cmd);
			deallocate_message(cont_byte_len);
			deallocate_message(user_cmd);
			deallocate_message(con_buf);
			deallocate_message(protocol_msg);
			deallocate_message(protocol_resp);
			deallocate_message(f_name);
			deallocate_message(f_content);
			deallocate_message(cts_or_err);
		}
	}
	return 0; // shouldnt ever run
}
