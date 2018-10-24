#include "InBand_FTP.h"

const int GLOB_BUF_LEN = 512; // global variable to allocate different amounts of chars
const int GLOB_FILE_LEN = 80000;
const int ACC_CONN = 10; // global variable for amount of acceptable connections

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
	char* con_buf = (char*)malloc(GLOB_BUF_LEN); // allocate message memory
	memset(con_buf, '\0', GLOB_BUF_LEN); // clear the "con_buf" buffer

	if(con == NULL) // dont allow empty struct
	{
		return "ERR:999 connection info is empty.\n";
	}

	recv(con->socket, con_buf, GLOB_BUF_LEN, 0); // receive the message

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
			char* user_cmd = (char*)malloc(GLOB_BUF_LEN); // "user_cmd" buffer ; "PMSG:filename.ext"
			memset(user_cmd, '\0', GLOB_BUF_LEN); // clear the user_cmd buffer

			char* con_buf = (char*)malloc(GLOB_BUF_LEN); // "ConnectionInfo" buffer ; 
			memset(con_buf, '\0', GLOB_BUF_LEN); // clear the buffer

			char* protocol_msg = (char*)malloc(GLOB_BUF_LEN); // protocol message buffer ; "PMSG:"
			memset(protocol_msg, '\0', GLOB_BUF_LEN); // clear the protocol message buffer

			char* protocol_resp = (char*)malloc(GLOB_BUF_LEN); // protocol response buffer ; "CTS:filename.ext" ; "CONT:f_byte_len:content"
			memset(protocol_resp, '\0', GLOB_BUF_LEN); // clear the protocol response buffer

			char* f_name = (char*)malloc(GLOB_BUF_LEN); // file name buffer ; filename.ext
			memset(f_name, '\0', GLOB_BUF_LEN); // clear the "f_name" buffer

			char* f_content = (char*)malloc(GLOB_FILE_LEN); // file content buffer
			memset(f_content, '\0', GLOB_FILE_LEN); // clear the file content buffer

			FILE* serv_file;

			// set up ConnectionInfo structure for the server
			con.socket = newsockfd;
			con.buf = con_buf;
			con.buf_length = GLOB_BUF_LEN;


			user_cmd = receiveMessage(&con);
			printf("user_cmd = %s\n", user_cmd); // PMSG:filename.ext

			// isolate user commands
			if((strcmp(user_cmd, "help") && strcmp(user_cmd, "quit"))) // skip if user sent "help" or "quit"
			{
				memcpy(protocol_msg, user_cmd, 5);
				printf("protocol_msg = %s\n", protocol_msg); // PMSG:
				memcpy(f_name, user_cmd + 5, GLOB_BUF_LEN-5);
				printf("f_name = %s\n", f_name); // filename.txt
			}

			if(!strcmp(user_cmd, "help")) // if client sends "help"
			{
				sendMessage(&con, "The avaliable FTP commands are: quit, get <filename>, & put <filename>.\n");
			}
			else if(!strcmp(user_cmd, "quit")) // if client sends "quit" command
			{
				sendMessage(&con, "Closing client connection...\n");
				closed = true;
				close(newsockfd);
			}
			else if(!strcmp(user_cmd, "no_file_in_cli_dir"))
			{
				printf("...failed STOR.\n\n");
				sendMessage(&con, "ERR:404 file does not exist in local directory.");
			}
			else if(!strcmp(user_cmd, "STOR:")) // if client sends "put" without filename
			{
				printf("...failed STOR.\n\n");
				sendMessage(&con, "ERR:XXX");
			}
			else if(!strcmp(user_cmd, "RETV:")) // if client sends "get" without filename
			{
				printf("...failed RETV.\n\n");
				sendMessage(&con, "ERR:XXX");
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
					memcpy(protocol_resp, "CTS:", 4);
					strcat(protocol_resp, f_name);
					printf("%s\n\n", protocol_resp);
					sendMessage(&con, protocol_resp);
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
			deallocate_message(user_cmd);
			deallocate_message(con_buf);
			deallocate_message(protocol_msg);
			deallocate_message(protocol_resp);
			deallocate_message(f_name);
			deallocate_message(f_content);
		}
	}
	return 0; // shouldnt ever run
}



