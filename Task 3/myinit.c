#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LINE_LEN 1024
#define MAX_PROCS_NUM 32

#define PATH_IS_NOT_ABSOLUTE_TEMPLATE "Path %s is not absolute\n"
#define OPEN_CONFIG_ERROR "Failed to open config file."
#define CHDIR_ERROR "Failed to change directory.\n"
#define START_DAEMON_MESSAGE "Start daemon\n"
#define FORK_ERROR_TEMPLATE "Fork error.\n"
#define FORK_SUCCESS_TEMPLATE "A new process with pid %d is forked from process %d\n"
#define CHILD_FINISHED_TEMPLATE "Child process with pid: %d finished\n"
#define SIGHUP_HANDLED_TEMPLATE "SIGHUP: Updating config, restarting processes\n"
#define TWO_ARGUMENTS_NEEDED_ERROR "Two arguments needed!\n"

pid_t pids[MAX_PROCS_NUM];
int pid_count;
char config_file_name[MAX_LINE_LEN];
FILE *log_file;

struct config {
    int executables_count;
    char path_to_executable[MAX_PROCS_NUM][MAX_LINE_LEN];
};

struct process {
    char **args;
    char in_path[MAX_LINE_LEN];
    char out_path[MAX_LINE_LEN];
};

void check_path_is_absolute(char *path){
    if (path[0] != '/') {
        printf(PATH_IS_NOT_ABSOLUTE_TEMPLATE, path);
        exit(1);
    }
}

char* remove_endline_sym(char *line) {
    unsigned long last_sym = strlen(line) - 1;
    if (line[last_sym] == '\n')
        line[strlen(line)-1] = '\0';
    return line;
}

char** add_line(char **array, char* buffer, int count){
    array = (char**)realloc(array, (count+1)*sizeof(*array));
    array[count-1] = (char*)malloc(MAX_LINE_LEN);
    strcpy(array[count-1], buffer);
    return array;
}

struct config read_config_file(char *config_filename){
    check_path_is_absolute(config_filename);
    FILE *config_file = fopen(config_filename, "r");
    if (config_file == NULL) {
        perror(OPEN_CONFIG_ERROR);
        exit(1);
    }

    char buffer[MAX_LINE_LEN];
    char **paths = NULL;
    int count = 0;
    struct config conf;

    while(fgets(buffer, MAX_LINE_LEN, config_file) != NULL) {
        count++;
        paths = add_line(paths, buffer, count);
    }

    for (int i = 0; i < count; i++) {
        strcpy(conf.path_to_executable[i], remove_endline_sym(paths[i]));
    }
    conf.executables_count = count;
    free(paths);
    fclose(config_file);
    return conf;
}

void check_conf_for_absolute_paths(struct config conf) {
    for (int i = 0; i < conf.executables_count; i++)
        check_path_is_absolute((char *) conf.path_to_executable[i]);
}

void change_directory_on_root() {
    char *new_dir = "/";

    if (chdir(new_dir) != 0) {
        printf(CHDIR_ERROR);
        exit(1);
    }
}

void close_streams() {
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
}

void daemonize() {
    if (getppid() != 1) {
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);

        if (fork())
            exit(0);

        setsid();
    }
}

FILE* open_log() {
    FILE *log = fopen("/tmp/myinit.log", "wa");
    fwrite(START_DAEMON_MESSAGE, 1, sizeof(START_DAEMON_MESSAGE) - 1, log);
    fflush(log);
    return log;
}

void write_log(char *message, FILE *log) {
    fwrite(message, 1, strlen(message), log);
    fflush(log);
}

struct process parse_args(char str[MAX_LINE_LEN]) {
    char buffer[MAX_LINE_LEN];
    strcpy(buffer, str);
    struct process proc;
    char **array = NULL;

    char *line_part = NULL;

    int counter = 1;
    for (; (line_part = strsep(&str, " ")); counter++) // counter делает 1 лишний шаг
        array = add_line(array, line_part, counter);

    strcpy(proc.in_path, array[counter - 3]);
    strcpy(proc.out_path, array[counter - 2]);

    char **args_array = NULL;
    char *arg_buffer = NULL;
    for (int i = 1; i <= counter - 3; i++) {
        arg_buffer = (char*) malloc(sizeof(array[i - 1]));
        strcpy(arg_buffer, array[i - 1]);
        args_array = add_line(args_array, arg_buffer, i);
    }
    proc.args = args_array;

    return proc;
}

void reopen_streams(struct process proc) {
    check_path_is_absolute(proc.in_path);
    check_path_is_absolute(proc.out_path);
    freopen(proc.in_path, "r", stdin);
    freopen(proc.out_path, "w", stdout);
}

pid_t start_proc(struct process proc, FILE *log, int task_number) {
    char buffer[MAX_LINE_LEN];
    reopen_streams(proc);
    pid_t cpid = fork();
    switch(cpid){
        case -1:
            write_log(FORK_ERROR_TEMPLATE, log);
            break;
        case 0:
            execv(proc.args[0], proc.args);
            break;
        default:
            sprintf(buffer, FORK_SUCCESS_TEMPLATE, cpid, getppid());
            write_log(buffer, log);
            pids[task_number] = cpid;
            pid_count++;
            break;
    }
    return cpid;
}

void run(struct config conf, FILE *log) {
    pid_t cpid;
    int p = 0;
    struct process proc[conf.executables_count];
    char buffer[MAX_LINE_LEN];

    for (; p < conf.executables_count; p++){
        proc[p] = parse_args(conf.path_to_executable[p]);
        start_proc(proc[p], log, p);
    }

    while(pid_count) {
        cpid = waitpid(-1, NULL, 0);
        for (p = 0; p < conf.executables_count; p++)
            if (pids[p] == cpid){
                sprintf(buffer, CHILD_FINISHED_TEMPLATE, cpid);
                write_log(buffer, log);
                pids[p] = 0;
                pid_count--;
                cpid = start_proc(proc[p], log, p);
            }
    }

    for (p = 0; p < conf.executables_count; p++)
        free(proc[p].args);
}

void sighup_handler(int sig){
    for (int p = 0; p < MAX_PROCS_NUM; p++)
        if (pids[p] != 0)
            kill(pids[p], SIGKILL);

    struct config conf = read_config_file(config_file_name);
    check_conf_for_absolute_paths(conf);
    write_log(SIGHUP_HANDLED_TEMPLATE, log_file);

    run(conf, log_file);
}

int main(int argc, char *argv[]){
    if (argc != 2) {
        printf(TWO_ARGUMENTS_NEEDED_ERROR);
        return 1;
    }
    strcpy(config_file_name, argv[1]);

    change_directory_on_root();
    close_streams();
    daemonize();
    signal(SIGHUP, sighup_handler);

    struct config conf = read_config_file(config_file_name);
    check_conf_for_absolute_paths(conf);
    log_file = open_log();

    run(conf, log_file);
    return 0;
}

