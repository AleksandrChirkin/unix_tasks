#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024
#define READ_MAX 10

#define THREE_ARGUMENTS_NEEDED_ERROR "Three arguments needed!\n"
#define CONFIG_NOT_OPENED_ERROR "Config file not opened!\n"
#define CONNECT_FAILED "Connect failed\n"

int handle_exception(char* error_msg){
    perror(error_msg);
    return 1;
}

void sigpipe_handler(int sig_num){
}

int main(int argc, char **argv) {
    if (argc != 3){
        perror(THREE_ARGUMENTS_NEEDED_ERROR);
        return 1;
    }

    signal(SIGPIPE, sigpipe_handler);

    FILE *config_file = fopen(argv[1], "r");
    if (config_file == NULL)
        return handle_exception(CONFIG_NOT_OPENED_ERROR);

    char sock_name[MAX_LINE_LENGTH];
    char *sock_name_buffer = (char*) malloc(MAX_LINE_LENGTH);
    fgets(sock_name_buffer, MAX_LINE_LENGTH, config_file);
    sprintf(sock_name, "/tmp/%s", sock_name_buffer);
    free(sock_name_buffer);

    int reads_num = atoi(argv[2]);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    addr.sun_family = AF_UNIX;

    strcpy(addr.sun_path, sock_name);
    socklen_t len = sizeof(addr);

    if (connect(sock, (struct sockaddr *)&addr, len) == -1)
        return handle_exception(CONNECT_FAILED);

    char *to_send = NULL;
    char *to_recv = NULL;
    for (int i = 0; i < reads_num; i++) {
        to_send = malloc(READ_MAX + 1);
        to_recv = malloc(READ_MAX + 1);
        scanf("%10s", to_send);

        write(sock, to_send, strlen(to_send));
        read(sock, to_recv, READ_MAX);

        free(to_send);
        free(to_recv);
    }

    close(sock);
    return 0;
}