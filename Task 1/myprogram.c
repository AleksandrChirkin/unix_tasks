#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 4096
#define INVALID_NUMBER_OF_ARGUMENTS_MSG "Error: Invalid number of arguments: %d\n"
#define ERROR_INPUT_FILE_MSG "Error: Failed to read from input file!\n"
#define ERROR_OUTPUT_FILE_MSG "Error: Can't create output file!\n"
#define BLOCK_SIZE_MSG "Error: Invalid block size (negative, zero or non-digit)\n"
#define READ_ERROR_MSG "read error\n"
#define WRITE_ERROR_MSG "write error\n"
#define LSEEK_ERROR_MSG "lseek error\n"
#define SUCCESS_MSG "Sparse file %s created successfully!\n"

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
    printf(msg);
    close_fds(input_fd, output_fd);
    return 1;
}

int main(int argc, char** argv) {
    int block_size = BLOCK_SIZE;
    char* sparse_file_name;
    int input_fd, second_arg;

    switch (argc) {
        case 2:
            input_fd = STDIN_FILENO;
            sparse_file_name = argv[1];
            break;
        case 4:
            input_fd = open_input(argv[1]);
            sparse_file_name = argv[2];
            block_size = atoi(argv[3]);
            break;
        case 3:
            second_arg = atoi(argv[2]);
            if (second_arg)
            {
                input_fd = STDIN_FILENO;
                sparse_file_name = argv[1];
                block_size = second_arg;
            }
            else
            {
                input_fd = open_input(argv[1]);
                sparse_file_name = argv[2];
            }
            break;
        default:
            printf(INVALID_NUMBER_OF_ARGUMENTS_MSG, argc - 1);
            return 1;
    }
    int output_fd = open_output(sparse_file_name);
    if (output_fd == -1) {
        return terminate_with_exception(ERROR_OUTPUT_FILE_MSG, input_fd, output_fd);
    }
    if (input_fd == -1)
    {
        return terminate_with_exception(ERROR_INPUT_FILE_MSG, input_fd, output_fd);
    }
    if (block_size <= 0)
    {
        return terminate_with_exception(BLOCK_SIZE_MSG, input_fd, output_fd);
    }
    char write_buffer[block_size];
    char read_buffer[block_size];
    int zero_bytes = 0;
    int write_bytes = 0;
    int i, write_result, lseek_result;
    while (block_size = read(input_fd, read_buffer, block_size)) {
        if (block_size == -1) {
            return terminate_with_exception(READ_ERROR_MSG, input_fd, output_fd);
        }
        i = 0;
        while (i < block_size) {
            for (; i < block_size && read_buffer[i] != 0; i++) {
                write_buffer[write_bytes] = read_buffer[i];
                write_bytes++;
            }
            if (write_bytes != 0) {
                write_result = write(output_fd, write_buffer, write_bytes);
                if (write_result == -1)
                {
                    return terminate_with_exception(WRITE_ERROR_MSG, input_fd, output_fd);
                }
                write_bytes = 0;
            }
            for (; i < block_size && read_buffer[i] == 0; i++) {
                zero_bytes++;
            }
            if (zero_bytes != 0) {
                lseek_result = lseek(output_fd, zero_bytes, SEEK_CUR);
                if (lseek_result == -1)
                {
                    return terminate_with_exception(LSEEK_ERROR_MSG, input_fd, output_fd);
                }
                zero_bytes = 0;
            }
        }
    }
    printf(SUCCESS_MSG, sparse_file_name);
    close_fds(input_fd, output_fd);
    return 0;
}
