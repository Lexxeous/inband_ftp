{
    src_fd = open(src_path, O_RDONLY);
    dst_fd = open(dst_path, O_CREAT | O_WRONLY);

    while (1) {
        err = read(src_fd, buffer, 4096);
        if (err == -1) {
            printf("Error reading file.\n");
            exit(1);
        }
        n = err;

        if (n == 0) break;

        err = write(dst_fd, buffer, n);
        if (err == -1) {
            printf("Error writing to file.\n");
            exit(1);
        }
    }

    close(src_fd);
    close(dst_fd);
}


// get the "f_content" from "f_name" in "server_dir" one byte at a time
char ch = fgetc(serv_file);
int ch_cnt = 0;
while (ch != EOF)
{
    ch_cnt += 1;
    append_char(f_content, ch);
    ch = fgetc(serv_file);
}
printf("\n");

// form and send the "CONT:<cont_byte_len>:<f_content>" message
itoa(ch_cnt, cont_byte_len, 10);
memcpy(protocol_msg, "CONT:", P_MSG_SIZE);
memcpy(cont_cmd, protocol_msg, P_MSG_SIZE);
strcat(cont_cmd, cont_byte_len);
append_char(cont_cmd, ':');
strcat(cont_cmd, f_content);
sendMessage(&con, cont_cmd);
