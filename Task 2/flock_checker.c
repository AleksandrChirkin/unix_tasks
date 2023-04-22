#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define STAT_TEMPLATE "%d: Successful locks number: %d\n"
#define LOCK_OPEN_ERROR_MSG "%d: An error occurred while opening lock file %s\n"
#define LOCK_WRITE_ERROR_MSG "%d: An error occurred while writing to lock file %s\n"
#define LOCK_READ_ERROR_MSG "%d: An error occurred while reading from lock file %s\n"
#define LOCK_BY_ANOTHER_PROCESS "%d: File %s has already been locked by another process with pid %s\n"
#define FILE_OPEN_ERROR_MSG "%d: An error occurred while opening file %s\n"
#define FILE_WRITE_ERROR_MSG "%d: An error occurred while writing to file %s\n"

int running = 1;
int write_stats = 0;

void sigintHandler(int sig_num) {
    running = 0;
}

void read_args(int argc, char** argv) {
    int c;
    while ((c = getopt (argc, argv, "s")) != -1)
    {
        if (c == 's')
            write_stats = 1;
        else {
            printf("Command format: %s [-s] file_name", argv[0]);
            exit(1);
        }
    }
}

int handle_file_exception(char* template, char* arg, int pid, int fd, int lock_fd) {
    fprintf(stderr, template, pid, arg);
    close(lock_fd);
    close(fd);
    return 1;
}

int get_int_length(int arg) {
    int length = 1;
    while (arg > 9) {
        arg /= 10;
        length++;
    }
    return length;
}

int main(int argc, char** argv) {
    signal(SIGINT, sigintHandler);

    read_args(argc, argv);

    char* file_name = argv[optind];
    char* lck_file_name = (char*) malloc((strlen(file_name) + 4) * sizeof(char));
    sprintf(lck_file_name, "%s.lck", file_name);

    int pid = getpid();
    int pid_len = get_int_length(pid);
    char *pid_bytes = (char*) malloc(pid_len);
    char *pid_bytes_with_nl = (char*) malloc(pid_len + sizeof(char));
    sprintf(pid_bytes, "%d", pid);
    sprintf(pid_bytes_with_nl, "%d\n", pid);

    int lock_fd, fd;
    ssize_t read_bytes, written;
    char read_buffer[pid_len];

    int successful_locks = 0;
    while(running) {
        lock_fd = -1;
        while (lock_fd == -1 && running) {
            lock_fd = open(lck_file_name, O_CREAT | O_EXCL | O_WRONLY, 0600);
            sleep(0.1);
        }
        if (!running) {
            close(lock_fd);
            break;
        }

        written = write(lock_fd, pid_bytes, pid_len);
        if (written == -1)
            return handle_file_exception(LOCK_WRITE_ERROR_MSG, lck_file_name, pid, fd, lock_fd);

        close(lock_fd);

        fd = open(file_name, O_CREAT | O_WRONLY | O_APPEND, 0777);
        if (fd == -1)
            return handle_file_exception(FILE_OPEN_ERROR_MSG, file_name, pid, fd, lock_fd);

        written = write(fd, pid_bytes_with_nl, pid_len + 1);
        if (written == -1)
            return handle_file_exception(FILE_WRITE_ERROR_MSG, file_name, pid, fd, lock_fd);

        close(fd);

        sleep(1);

        lock_fd = open(lck_file_name, O_RDONLY);
        if (lock_fd == -1)
            return handle_file_exception(LOCK_OPEN_ERROR_MSG, lck_file_name, pid, fd, lock_fd);

        read_bytes = read(lock_fd, read_buffer, pid_len);
        if (read_bytes == -1)
            return handle_file_exception(LOCK_READ_ERROR_MSG,  lck_file_name, pid, fd, lock_fd);

        if (strcmp(read_buffer, pid_bytes) != 0) {
            fprintf(stderr, LOCK_BY_ANOTHER_PROCESS, pid, file_name, read_buffer);
            return 1;
        }

        close(lock_fd);

        remove(lck_file_name);
        successful_locks++;
    }

    if (write_stats) {
        printf(STAT_TEMPLATE, pid, successful_locks);
    }

    return errno;
}
