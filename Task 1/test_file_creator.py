try:
    file_len = 4*1024*1024 + 1
    offsets = [0, 10000, file_len - 1]
    with open('fileA', mode='w') as test_file:
        for i in range(file_len):
            if i in offsets:
                test_file.write('\1')
            else:
                test_file.write('\0')
except Exception as e:
    print(f'Creation of test file failed due to exception {e}')
