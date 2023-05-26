log_to_result() {
  printf '%s %s\n' "$(date +'%d.%m.%y %H:%M:%S.%N')" "$1" >> result.txt
}

rm result.txt /tmp/server.log

log_to_result 'Compiling client and server'
make && log_to_result 'Client and server compiled successfully!'
chmod 700 client server test_client

log_to_result 'Generating numbers and config files'
python3 generator.py
( ./server config.txt >> result.txt ) &
sleep 1

log_to_result 'Running first test suite (performance tests) for 5 times'
for _ in {1..5}
do
  ./test1.sh >> result.txt
done
log_to_result 'First test suite complete! Log content (first and last lines): '
grep -in 'Socket 6 accepted. ' /tmp/server.log | sed -n '1p;$p'  >> result.txt

clients=(1 2 10 100)
delays=(0 0.2 0.4 0.6 0.8 1)  # попытаться как-то через цикл с шагом

log_to_result 'Running second test suite (efficiency tests) for different number of clients and delays'
for client in ${clients[@]}
do
    for delay in ${delays[@]}
    do
        log_to_result "$client clients; Delay = $delay"
        ./test2.sh $client $delay
    done
done
log_to_result 'Second test suite complete!'
killall server