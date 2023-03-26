rm file*
printf '\nCreating test file A\n'
python3 test_file_creator.py && printf 'Test file created successfully!\n'
printf '\nCompiling sparse file creator\n'
make myprogram && printf 'Sparse file creator compiled successfully!\n'
chmod 700 myprogram
printf '\nTransforming file A into sparse file B\n'
./myprogram fileA fileB
printf '\nCompressing A and B\n'
gzip -k fileA fileB && printf 'A and B files compressed successfully!\n'
printf '\nDecompressing file B and transform decompressed file into sparse file C\n'
gzip -cd fileB.gz | ./myprogram fileC
printf '\nTransforming file A into file B with block size 100\n'
./myprogram fileA fileD 100
printf '\nPrinting size and number of blocks of files A, A.gz, B, B.gz, C, D\n'
stat --format='%n %s %b' file*