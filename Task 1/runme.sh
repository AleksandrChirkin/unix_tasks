rm file* result.txt

log_to_result() {
  printf '%s %s\n' "$(date +'%d.%m.%y %H:%M:%S.%N')" "$1" >> result.txt
}

log_to_result 'Creating test file A'
python3 test_file_creator.py && log_to_result 'Test file created successfully!'

log_to_result 'Compiling sparse file creator'
make sparse_file_creator && log_to_result 'Sparse file creator compiled successfully!'
chmod 700 sparse_file_creator

log_to_result 'Transforming file A into sparse file B'
./sparse_file_creator fileA fileB && log_to_result 'Sparse file B created successfully!'

log_to_result 'Compressing A and B'
gzip -k fileA fileB && log_to_result 'A and B files compressed successfully!'

log_to_result 'Decompressing file B and transform decompressed file into sparse file C'
(gzip -cd fileB.gz | ./sparse_file_creator fileC) && log_to_result 'Sparse file C created successfully!'

log_to_result 'Transforming file A into file B with block size 100'
./sparse_file_creator fileA fileD 100 && log_to_result 'Sparse file D created successfully!'

log_to_result 'Printing size and number of blocks of files A, A.gz, B, B.gz, C, D'
stat --format='%n %s %b' file*