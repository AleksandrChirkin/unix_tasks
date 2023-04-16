log_to_result() {
  printf '%s %s\n' "$(date +'%d.%m.%y %H:%M:%S.%N')" "$1" >> result.txt
}

rm -f stats.txt result.txt myfile*
touch myfile

log_to_result 'Compiling file lock checker'
make && log_to_result 'File lock checker compiled successfully!'
chmod 700 flock_checker

log_to_result 'Checking that lock fails after deleting .lck file'
( ./flock_checker myfile ) &
( ./flock_checker myfile ) &
rm -f myfile.lck
sleep 2
killall -SIGINT flock_checker
log_to_result 'Printing contested file content'
cat myfile >> result.txt

log_to_result 'Checking lock for 10 parallel tasks'
for _ in {1..10}
do
 ( ./flock_checker -s myfile ) &
done
sleep 300
log_to_result 'Killing tasks with SIGINT'
killall -SIGINT flock_checker && log_to_result 'Killed'