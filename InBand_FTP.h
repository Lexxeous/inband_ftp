#ifndef INBAND_FTP_H
#define INBAND_FTP_H

#include <stdio.h> // for file I/O
#include <stdlib.h>
#include <stdbool.h> // for boolean datatypes
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <math.h>


struct ConnectionInfo
{
	int socket;
	char* buf;
	int buf_length;
};
typedef struct ConnectionInfo ConnectionInfo;


// This function will send the message to the server.
// Param "msg" is a null terminated string to be sent to the server and its length may be arbitrarily long and is NOT limited to a few kilobytes.
// Param "con" contains the information required to send to the server. Note that this does mean that if there are multiple ConnectionInfo data structures this may not be the same server between calls.
// If the send was successful return 0. If it was unsuccessful for any reason then a non 0 value is returned. 
// Param "msg" or "con" being null is a sufficient condition for failure.
int sendMessage(struct ConnectionInfo* con, char* msg);


// Param "con" contains the information required to receive from the server. 
// Note that this does mean that if there are multiple ConnectionInfo data structures this may not be the same server between calls.
// This return value is null if an error has occurred. 
// Otherwise the returned value is a null terminated string that contains the entire message and thus must be able to hold a large amount of data (more than a few kilobytes) and is most easily accomplished by using the heap to allocate the memory. 
// This memory should not be modified by any further actions in the client library before deallocate_message is called on the returned value.
char* receiveMessage(struct ConnectionInfo* con);


// This function will deallocate the memory returned by receiveMessage. 
// This may be as simple as a wrapper to Cs free function or C++s delete. 
// If the value passed in mem did not come from receiveMessage then the results are undefined.
void deallocate_message(char* mem);


// This function will connect to a server and initilize the con data structure so that it can be used in future calls to sendMessage.
// Param "who" is the name of the host as a null terminated ASCII string.
// Param "port" is the port the server is running on.
// Param "con" will contain after this call, if it is successful, the required information to be used in the sendMessage and receiveMessage functions.
// If the connection attempt was successful 0 is returned. If the connection attempt was unsuccessful then a non 0 value is returned. 
// Param "who" or "con" being null is a sufficient condition for failure. 
// In the event of a failure the state of the system and the arguments is such that another attempt can be made, for instance with a new port number.
int connect_to_server(char* who, int port, struct ConnectionInfo* con);

 
// Param "port" is a port number to bind to.
// If the bind was successful then the server is now running on a new thread and 0 is returned.
// If it was unsuccessful a nonzero value is returned. After an unsuccessful it must be possible to reattempt the bind and have it succeed if the operating system will allow the program to bind to the requested port number.
int run_server(int port);


void append_char(char* s, char c);


/*
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
*/
char* itoa(int value, char* result, int base);


void write_to_new_file(const char* filepath, const char* data);


char* extract_byte_len(char* s);


int int_width(int x);


bool is_empty_file(const char* filepath);


#endif