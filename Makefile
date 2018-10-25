# set default variables
tcp_port?=10001
server_port?=10001
server_name?=localhost

run_s: ftp_server
	cd server_dir && ./ftp_server $(server_port) # run server executable on particular port

run_c: ftp_client
	cd client_dir && ./ftp_client $(server_name) $(server_port) # run client executable with server name and port number

ftp_server:
	gcc -o ftp_server ftp_server.c InBand_FTP.c # compile and link into a ftp_server executable
	mv ftp_server server_dir

ftp_client:
	gcc -o ftp_client ftp_client.c InBand_FTP.c # compile and link into a ftp_client executable
	mv ftp_client client_dir

clean:
	cd server_dir && rm ftp_server # remove ftp_server executable
	# returns to the orignal directory between commands by default
	cd client_dir && rm ftp_client # remove ftp_client executable

pid:
	lsof -i tcp:$(tcp_port)

pids:
	lsof -i tcp:10001
	lsof -i tcp:10002
	lsof -i tcp:10003

kill_s:
	kill `pgrep ftp_server` # kill <server_PID>

kill_c:
	kill `pgrep ftp_client` # kill <client_PID>
