for _ in {1..100}
do
    ( ( cat numbers.txt | ./test_client config.txt 1000 0 ) > /dev/null ) &
done
wait
echo 0 | ./client config.txt 1