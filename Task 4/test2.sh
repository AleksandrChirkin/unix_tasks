clients_num=$1
delay=$2

rm -f /tmp/client.log
touch /tmp/client.log
SECONDS=0
start_time="$(date -u +%s)"

for ((i=1; i <= clients_num; i++));
do
    ( (./test_client config.txt 20 $delay < numbers.txt) >> /tmp/client.log ) &
done
wait
echo "Results for $clients_num clients and delay for $delay sec:" >> result.txt
slowest_client=$(grep -Eo '[0-9]+' /tmp/client.log | sort -rn | head -n 1)
echo "Performance of the slowest client is $slowest_client seconds" >> result.txt

end_time="$(date -u +%s)"
elapsed="$(($end_time-$start_time))"
echo "Time elapsed: $elapsed sec" >> result.txt

server_duration=$SECONDS
diff=$((server_duration - slowest_client))
echo "Difference between the slowest client and server is $diff seconds " >> result.txt