#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define MAX_LINE_LENGTH 1024
#define READ_MAX 10

#define FOUR_ARGUMENTS_NEEDED_ERROR "Four arguments needed!\n"
#define CONFIG_NOT_OPENED_ERROR "Failed to open config file.\n"
#define CONNECT_FAILED "Connect failed\n"

void sigpipe_handler(int sig_num){
}

int handle_exception(char * message){
    perror(message);
    return 1;
}

int main(int argc, char **argv) {
    if (argc != 4)
        return handle_exception(FOUR_ARGUMENTS_NEEDED_ERROR);

    socklen_t len;
    struct sockaddr_un address;
    int result;

    signal(SIGPIPE, sigpipe_handler);

    int num_reads = atoi(argv[2]);
    float sleep_time = atof(argv[3]) * 1000000;

    FILE *config_file = fopen(argv[1], "r");
    if (config_file == NULL)
        return handle_exception(CONFIG_NOT_OPENED_ERROR);

    char socket_name[MAX_LINE_LENGTH];
    char *name_buf = malloc(MAX_LINE_LENGTH);char buffer[MAX_LINE_LENGTH];
    fgets(buffer, MAX_LINE_LENGTH, config_file);
    strcpy(name_buf, buffer);
    sprintf(socket_name, "/tmp/%s", name_buf);

    memset(&address, 0, sizeof(struct sockaddr_un));
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    address.sun_family = AF_UNIX;

    strcpy(address.sun_path, socket_name);
    len = sizeof(address);

    FILE *file = fopen("bigfile", "r");
    result = connect(sock, (struct sockaddr *)&address, len);

    if (result == -1)
        return handle_exception(CONNECT_FAILED);
    int seed;
    fread(&seed, sizeof(int), 1, file);
    srand(seed);
    time_t start_time = time(0);

    char *to_send;
    char *to_recv;
    char *big_file;
    for(int i = 0; i < num_reads; i++) {
        to_recv = malloc(READ_MAX + 1);
        to_send = malloc(READ_MAX + 1);
        scanf("%10s", to_send);
        write(sock, to_send, strlen(to_send));
        read(sock, to_recv, READ_MAX);

        int n = (rand() % 255) + 1;
        big_file = malloc(n);
        fread(big_file, sizeof(char), n, file);
        usleep(sleep_time);

        free(big_file);
        free(to_send);
        free(to_recv);
    }
    time_t end_time = time(0);

    printf("%f\n", difftime(end_time, start_time));
    close(sock);
    free(name_buf);

    return 0;
}