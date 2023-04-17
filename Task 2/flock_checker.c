#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

#define STAT_TEMPLATE "Successful locks number for pid %d: %d\n"
#define LOCK_OPEN_ERROR_MSG "An error occurred while opening lock file %s\n"
#define LOCK_WRITE_ERROR_MSG "An error occurred while writing to lock file %s\n"
#define LOCK_READ_ERROR_MSG "An error occurred while reading from lock file %s\n"
#define LOCK_BY_ANOTHER_PROCESS "File %s has already been locked by another process with pid %s (current pid=%s)\n"
#define FILE_OPEN_ERROR_MSG "An error occurred while opening file %s\n"
#define FILE_WRITE_ERROR_MSG "An error occurred while writing to file %s\n"
#define SIGINT_ERROR "An error with code %d occurred while handling SIGINT\n"

static volatile int running = 1;
static volatile int pid;
static volatile int successful_locks = 0;
static volatile int locked = 0;
static volatile int write_stats = 0;

void sigintHandler(int sig_num) {
    errno = 0;
    running = 0;
    if (write_stats) {
        int stat_fd = open("stats.txt", O_CREAT | O_WRONLY | O_APPEND, 0600);
        int locks_num_buffer_len = (int) ((ceil(log10(pid)) + ceil(log10(successful_locks)) + 35) * sizeof(char));
        char locks_num_buffer[locks_num_buffer_len];
        sprintf(locks_num_buffer, STAT_TEMPLATE, pid, successful_locks + locked);
        write(stat_fd, locks_num_buffer, locks_num_buffer_len);
        close(stat_fd);
        if (opterr > 0) {
            fprintf(stderr, SIGINT_ERROR, errno);
        }
    }
}

int read_args(int argc, char** argv) {
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "s")) != -1)
        switch (c)
        {
            case 's':
                write_stats = 1;
                break;
            case '?':
                if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,"Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort ();
        }
}

int handle_file_exception(char* template, char* arg, int fd, int lock_fd) {
    fprintf(stderr, template, arg);
    close(lock_fd);
    close(fd);
    return 1;
}

int main(int argc, char** argv) {
    signal(SIGINT, sigintHandler);

    read_args(argc, argv);

    char* file_name = argv[optind];
    char* lck_file_name = (char*) malloc((strlen(file_name) + 4) * sizeof(char));
    sprintf(lck_file_name, "%s.lck", file_name);

    pid = getpid();
    int pid_len = (int) ((ceil(log10(pid))) * sizeof(char));
    char *pid_bytes = (char*) malloc(pid_len);
    char *pid_bytes_with_nl = (char*) malloc(pid_len + sizeof(char));
    sprintf(pid_bytes, "%d", pid);
    sprintf(pid_bytes_with_nl, "%d\n", pid);

    int lock_fd, fd, written, read_bytes;
    char read_buffer[pid_len];

    while(running) {
        lock_fd = -1;
        while (lock_fd == -1 && running) {
            lock_fd = open(lck_file_name, O_CREAT | O_EXCL | O_WRONLY, 0600);
            sleep(0.1);
        }
        if (!running) {
            close(lock_fd);
            remove(lck_file_name);
            break;
        }

        written = write(lock_fd, pid_bytes_with_nl, pid_len);
        if (written == -1)
            return handle_file_exception(LOCK_WRITE_ERROR_MSG, lck_file_name, fd, lock_fd);

        close(lock_fd);
        locked = 1;

        fd = open(file_name, O_WRONLY | O_APPEND);
        if (fd == -1)
            return handle_file_exception(FILE_OPEN_ERROR_MSG, file_name, fd, lock_fd);

        written = write(fd, pid_bytes_with_nl, pid_len + 1);
        if (written == -1)
            return handle_file_exception(FILE_WRITE_ERROR_MSG, file_name, fd, lock_fd);

        close(fd);

        sleep(1);

        lock_fd = open(lck_file_name, O_RDONLY);
        if (lock_fd == -1)
            return handle_file_exception(LOCK_OPEN_ERROR_MSG, lck_file_name, fd, lock_fd);

        read_bytes = read(lock_fd, read_buffer, pid_len);
        if (read_bytes == -1)
            return handle_file_exception(LOCK_READ_ERROR_MSG, lck_file_name, fd, lock_fd);

        if (strcmp(read_buffer, pid_bytes)) {
            fprintf(stderr, LOCK_BY_ANOTHER_PROCESS, file_name, read_buffer, pid_bytes);
            return 1;
        }

        close(lock_fd);

        remove(lck_file_name);
        locked = 0;
        successful_locks++;
    }

    return errno;
}
