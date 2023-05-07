rm -rf /tmp/myinit
mkdir /tmp/myinit
touch /tmp/myinit/in /tmp/myinit/out

echo /bin/sleep 5 /tmp/myinit/in /tmp/myinit/out > /tmp/myinit/config
echo /bin/sleep 6 /tmp/myinit/in /tmp/myinit/out >> /tmp/myinit/config
echo /bin/sleep 7 /tmp/myinit/in /tmp/myinit/out >> /tmp/myinit/config

make myinit
chmod 700 myinit

./myinit /tmp/myinit/config

echo First three tasks: > result.txt
ps -ef | grep /bin/sleep >> result.txt

pkill -f "/bin/sleep 6"

sleep 1
echo After kill second task: >> result.txt
ps -ef | grep /bin/sleep >> result.txt

echo /bin/sleep 50 > /tmp/myinit/config
echo /tmp/task3/in >> /tmp/myinit/config
echo /tmp/task3/out >> /tmp/myinit/config
sleep 7

pkill --signal SIGHUP -f "./myinit /tmp/myinit/config"

echo After sending sighup to daemon: >> result.txt
ps -ef | grep /bin/sleep >> result.txt

echo Logs from file: >> result.txt
cat /tmp/myinit.log >> result.txt

pkill -f "./myinit /tmp/myinit/config"
