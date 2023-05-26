#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>

#define MAX_STR_LEN 1024
#define LISTEN_SIZE 128
#define MAX_CLIENTS_NUM 100
#define READ_MAX 10

#define TWO_ARGUMENTS_NEEDED_ERROR "Two arguments needed!\n"
#define CONFIG_NOT_OPENED_ERROR "Config file not opened!\n"
#define PATH_IS_TOO_LONG_ERROR_TEMPLATE "Socket path is too long: %s\n"
#define SERVER_STARTED_MESSAGE "Server started\n"
#define SOCKET_NOT_OPENED_ERROR "Socket not opened!\n"
#define FAILED_TO_MAKE_SOCKET_REUSABLE "Failed to make socket reusable!\n"
#define SOCKET_NAME_FOUND_MESSAGE "Socket name found\n"
#define BINDING_ERROR "Failed to bind socket!\n"
#define SOCKET_BOUND_MESSAGE "Socket bound\n"
#define SERVER_INITIALIZED_MESSAGE "Server initialized!\n"
#define SELECT_ERROR "Failed to make select on sockets!\n"
#define ACCEPT_ERROR "Failed to accept socket!\n"
#define ACCEPTING_SOCKET_MESSAGE_TEMPLATE "Accepting socket %d\n"
#define SOCKET_ACCEPTED_MESSAGE_TEMPLATE "Socket %d accepted. sbrk returns %li \n"
#define SOCKET_DISCONNECTED_MESSAGE_TEMPLATE "Socket %d disconnected\n"
#define RECEIVED_NUMBER_MESSAGE_TEMPLATE "Received number %s from client\n"
#define SENDING_STATE_MESSAGE_TEMPLATE "Sending state %d to client\n"

void write_log(char *message, FILE *log){
    fwrite(message, 1, strlen(message), log);
    fflush(log);
}

int handle_exception(char* error_msg, FILE* log){
    write_log(error_msg, log);
    perror(error_msg);
    return 1;
}

int main(int argc, char **argv){
    if (argc != 2) {
        perror(TWO_ARGUMENTS_NEEDED_ERROR);
        return 1;
    }

    FILE *log = fopen("/tmp/server.log", "w");

    FILE *config_file = fopen(argv[1], "r");
    if (config_file == NULL)
        return handle_exception(CONFIG_NOT_OPENED_ERROR, log);

    char sock_name[MAX_STR_LEN];
    char *sock_name_buffer = (char*) malloc(MAX_STR_LEN);
    fgets(sock_name_buffer, MAX_STR_LEN, config_file);
    sprintf(sock_name, "/tmp/%s", sock_name_buffer);
    free(sock_name_buffer);
    unlink(sock_name);

    struct sockaddr_un address;
    if (strlen(sock_name) > sizeof(address.sun_path) - 1) {
        char err_buf[strlen(PATH_IS_TOO_LONG_ERROR_TEMPLATE) - 2 + MAX_STR_LEN];
        sprintf(err_buf, PATH_IS_TOO_LONG_ERROR_TEMPLATE, sock_name);
        write_log(err_buf, log);
        return 1;
    }

    write_log(SERVER_STARTED_MESSAGE, log);

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
        return handle_exception(SOCKET_NOT_OPENED_ERROR, log);
    int option = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option)) == -1)
        return handle_exception(FAILED_TO_MAKE_SOCKET_REUSABLE, log);

    write_log(SOCKET_NAME_FOUND_MESSAGE, log);

    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, sock_name, sizeof(address.sun_path) - 1);

    if (bind(sock, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) == -1)
        return handle_exception(BINDING_ERROR, log);
    write_log(SOCKET_BOUND_MESSAGE, log);

    listen(sock, LISTEN_SIZE);
    write_log(SERVER_INITIALIZED_MESSAGE, log);

    int num, max_fd, fd, selector, accepted;
    int *clients = malloc(MAX_CLIENTS_NUM * sizeof(int));
    fd_set fds_to_read;
    char *buf;
    char *num_from;
    char *num_to;
    long sbrk_value;
    int state = 0;
    while (1) {
        FD_ZERO(&fds_to_read);
        FD_SET(sock, &fds_to_read);
        max_fd = sock;
        
        for (int i = 0; i < MAX_CLIENTS_NUM; i++){
            fd = clients[i];
            if (fd > 0)
                FD_SET(fd, &fds_to_read);
            if (fd > max_fd)
                max_fd = fd;
        }

        selector = select(max_fd + 1, &fds_to_read, NULL, NULL, NULL);
        if (selector < 0 && errno != EINTR)
            return handle_exception(SELECT_ERROR, log);

        if (FD_ISSET(sock, &fds_to_read)) {
            if ((accepted = accept(sock, NULL, NULL)) == -1)
                return handle_exception(ACCEPT_ERROR, log);

            buf = malloc(64);
            sprintf(buf, ACCEPTING_SOCKET_MESSAGE_TEMPLATE, accepted);
            write_log(buf, log);
            printf(ACCEPTING_SOCKET_MESSAGE_TEMPLATE, accepted);
            free(buf);

            for (int i = 0; i < MAX_CLIENTS_NUM; i++){
                if (!clients[i]) {
                    clients[i] = accepted;
                    buf = malloc(64);
                    sbrk_value = (long) sbrk(0);
                    sprintf(buf, SOCKET_ACCEPTED_MESSAGE_TEMPLATE, accepted, sbrk_value);
                    write_log(buf, log);
                    free(buf);
                    break;
                }
            }
        }

        for(int i = 0; i < MAX_CLIENTS_NUM; i++) {
            fd = clients[i];
            num_from = malloc(READ_MAX);
            num_to = malloc(READ_MAX);

            if (FD_ISSET(fd, &fds_to_read)) {
                if (read(fd, num_from, READ_MAX) == 0) {
                    buf = malloc(64);
                    sprintf(buf, SOCKET_DISCONNECTED_MESSAGE_TEMPLATE, fd);
                    write_log(buf, log);
                    printf(SOCKET_DISCONNECTED_MESSAGE_TEMPLATE, fd);
                    free(buf);
                    close(fd);
                    clients[i] = 0;
                } else {
                    buf = malloc(64);
                    sprintf(buf, RECEIVED_NUMBER_MESSAGE_TEMPLATE, num_from);
                    write_log(buf, log);
                    free(buf);
                    num = atoi(num_from);

                    state += num;
                    buf = malloc(64);
                    sprintf(num_to, "%d", state);
                    write(fd, num_to, READ_MAX);
                    sprintf(buf, SENDING_STATE_MESSAGE_TEMPLATE, state);
                    write_log(buf, log);
                    free(buf);
                }
            }
            free(num_from);
            free(num_to);
        }
    }

    close(sock);
    return 0;
}