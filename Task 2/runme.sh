log_to_result() {
  printf '%s %s\n' "$(date +'%d.%m.%y %H:%M:%S.%N')" "$1" >> result.txt
}

rm stats.txt result.txt
touch myfile

log_to_result 'Compiling file lock checker'
make && log_to_result 'File lock checker compiled successfully!'
chmod 700 flock_checker

( ./flock_checker myfile ) &
( ./flock_checker myfile ) &
rm myfile.lck
sleep 2
killall -SIGINT flock_checker

for _ in {1..10}
do
 ( ./flock_checker myfile ) &
done
sleep 300
killall -SIGINT flock_checker