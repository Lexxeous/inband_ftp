CSC 5200-002
Computer Networks, Fall 2018
Programming Assignment 2: InBand_FTP
JONATHAN ALEXANDER GIBSON
jagibson44
T00198998


INSTRUCTIONS:
1. Navigate to the root directory of the project folder.
2. Open 2 terminal windows in the root dirctory of the project folder.
3. Execute "make run_s" (with default 'make' values) to spin up the server in one of the terminal windows.
	3.1. Or, execute "make run_s server_port=<port_num>" to spin up the server in one of the terminal windows with custom 'make' values.
4. Take note of the port number (<port_num>) that the server is running on (10001 ~ 10101).
5. Open the other terminal window and execute "make run_c" to start and connect the client (with default 'make' values).
	5.1. Or, execute "make run_c server_name=<name> server_port=<port_num>" to start and connect the client with custom 'make' values.
6. On the client side, use the available FTP commands: help, quit, get <filename>, & put <filename> and observe the behavior.
	6.1. "help" will display the available FTP commands for reference.
	6.2. "quit" will terminate the client conenction and leave the server running.
	6.3. "get <filename>" will retrieve <filename> from "server_dir" and copy it to "client_dir" if possible and allowed.
	6.4. "put <filename>" will store/copy <filename> from "client_dir" into "server_dir" if possible and allowed.
	6.5. The FTP protocol will not allow the transfer of duplicately named files or empty files.
7. Terminate both the server and the client by typing `CTRL+c` in the client window.
	7.1. Or, execute "make kill_s" and "make kill_c" to terminate the server and client respectively.
8. Execute "make clean" to remove the server and client executables from their directories.


ERROR CODES:
ERR:000 empty client file.
ERR:000 empty server file.

ERR:400 bad request, FTP command too short.
ERR:400 bad request, improper command format and filename not included for storage.
ERR:400 bad request, filename not included for storage.
ERR:400 bad request, improper command format and filename not included for retrieval.
ERR:400 bad request, filename not included for retrieval.

ERR:403 forbidden, improper command format.

ERR:404 <filename> does not exist in the local client directory.
ERR:404 <filename> does not exist in the remote server directory.

ERR:409 conflict, file already exists in remote directory.

ERR:503 bad sequence of commands, improper client command.
ERR:503 bad sequence of commands, improper server response.

ERR:999 connection info is empty.