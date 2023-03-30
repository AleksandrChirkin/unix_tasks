#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 4096
#define INVALID_NUMBER_OF_ARGUMENTS_MSG "Error: Invalid number of arguments: %d\n"
#define ERROR_INPUT_FILE_MSG "Error: Failed to read from input file!\n"
#define ERROR_OUTPUT_FILE_MSG "Error: Failed to write to output file!\n"
#define BLOCK_SIZE_ERROR_MSG "Error: Invalid block size (negative, zero or non-digit)\n"
#define LSEEK_ERROR_MSG "lseek error\n"

int open_input(char* file){
    return open(file, O_RDONLY);
}

int open_output(char* file){
    return open(file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
}

void close_fds(int input_fd, int output_fd){
    close(input_fd);
    close(output_fd);
}

int terminate_with_exception(char* msg, int input_fd, int output_fd){
    perror(msg);
    close_fds(input_fd, output_fd);
    return 1;
}

int main(int argc, char** argv) {
    int block_size = BLOCK_SIZE;
    int input_fd, output_fd, second_arg;

    switch (argc) {
        case 2:
            input_fd = STDIN_FILENO;
            output_fd = open_output(argv[1]);
            break;
        case 4:
            input_fd = open_input(argv[1]);
            output_fd = open_output(argv[2]);
            block_size = atoi(argv[3]);
            break;
        case 3:
            second_arg = atoi(argv[2]);
            if (second_arg || !strcmp(argv[2], "0")) {
                input_fd = STDIN_FILENO;
                output_fd = open_output(argv[1]);
                block_size = second_arg;
            }
            else {
                input_fd = open_input(argv[1]);
                output_fd = open_output(argv[2]);
            }
            break;
        default:
            fprintf(stderr, INVALID_NUMBER_OF_ARGUMENTS_MSG, argc - 1);
            return 1;
    }
    if (input_fd == -1) {
        return terminate_with_exception(ERROR_INPUT_FILE_MSG, input_fd, output_fd);
    }
    if (output_fd == -1) {
        return terminate_with_exception(ERROR_OUTPUT_FILE_MSG, input_fd, output_fd);
    }
    if (block_size <= 0) {
        return terminate_with_exception(BLOCK_SIZE_ERROR_MSG, input_fd, output_fd);
    }
    char buffer[block_size];
    int is_block_empty;
    int i, write_result, lseek_result;
    while (block_size = read(input_fd, buffer, block_size)) {
        if (block_size == -1) {
            return terminate_with_exception(ERROR_INPUT_FILE_MSG, input_fd, output_fd);
        }
        is_block_empty = 1;
        for (i = 0; i < block_size; i++) {
            if (buffer[i] != 0)
                is_block_empty = 0;
        }
        if (is_block_empty) {
            lseek_result = lseek(output_fd, block_size, SEEK_CUR);
            if (lseek_result == -1)
            {
                return terminate_with_exception(LSEEK_ERROR_MSG, input_fd, output_fd);
            }
        } else {
            write_result = write(output_fd, buffer, block_size);
            if (write_result == -1)
            {
                return terminate_with_exception(ERROR_OUTPUT_FILE_MSG, input_fd, output_fd);
            }
        }
    }
    close_fds(input_fd, output_fd);
    return 0;
}
