#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>

static volatile int running = 1;
static volatile int pid;
static volatile int successful_locks = 0;

void sigintHandler(int sig_num){
    running = 0;
    int stat_fd = open("stats.txt", O_CREAT | O_WRONLY | O_APPEND, 0600);
    int locks_num_buffer_len = (int) ((ceil(log10(pid)) + ceil(log10(successful_locks)) + 36) * sizeof(char));
    char locks_num_buffer[locks_num_buffer_len];
    sprintf(locks_num_buffer, "Successful locks number for pid %d: %d\n", pid, successful_locks);
    write(stat_fd, locks_num_buffer, locks_num_buffer_len);
    close(stat_fd);
}

int main(int argc, char** argv) {
    signal(SIGINT, sigintHandler);
    char* file_name = argv[1];
    char* lck_file_name = (char*) malloc((strlen(file_name) + 4) * sizeof(char));
    sprintf(lck_file_name, "%s.lck", file_name);
    int lock_fd, fd;
    pid = getpid();
    int pid_len = (int) ((ceil(log10(pid))) * sizeof(char));
    while(running) {
        lock_fd = -1;
        while (lock_fd == -1) {
            lock_fd = open(lck_file_name, O_CREAT | O_EXCL | O_WRONLY, 0200);
        }

        char *pid_bytes = (char*) malloc(pid_len);
        sprintf(pid_bytes, "%d", pid);
        write(lock_fd, pid_bytes, pid_len);
        close(lock_fd);

        fd = open(file_name, O_RDWR);
        write(fd, pid_bytes, pid_len);
        close(fd);

        sleep(1);
        remove(lck_file_name);
        successful_locks++;
    }
}
