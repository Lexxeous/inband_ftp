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





printf("HERE0\n");
printf("HERE1\n");
printf("HERE2\n");
printf("HERE3\n");
printf("HERE4\n");
printf("HERE5\n");
printf("HERE6\n");
printf("HERE7\n");
printf("HERE8\n");
printf("HERE9\n");


bool is_empty_file(const char* filepath);
{
    FILE* fd = fopen(f_name, "r")
    if (fd != NULL) 
    {
        fseek (fd, 0, SEEK_END);
        size = ftell(fd);

        if (0 == size)
        {
            return true;
        }
    }
}
