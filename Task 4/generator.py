#!/usr/bin/env python3
from uuid import uuid4
from random import randint


with open('config.txt', mode='w') as config:
    config.write(str(uuid4()))
num_sum = 0
max_ten_digits_num = 9999999999
while True:
    num_sum = 0
    with open('numbers.txt', mode='w') as nums:
        for _ in range(999):
            next_num = randint(-max_ten_digits_num, max_ten_digits_num)
            num_sum += next_num
            nums.write(f'{next_num}\n')
        if abs(num_sum) < max_ten_digits_num:
            nums.write(f'{-num_sum}')
            break
with open('bigfile', mode='w') as big_file:
    for _ in range(2 ** 18):
        sym = chr(randint(0, 256*128-1))
        big_file.write(sym)
