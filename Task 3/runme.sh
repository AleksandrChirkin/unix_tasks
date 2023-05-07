log_to_result() {
  printf '%s %s\n' "$(date +'%d.%m.%y %H:%M:%S.%N')" "$1" >> result.txt
}

rm -rf /tmp/myinit result.txt

log_to_result "Creating temporary directory for myinit" daemon
mkdir /tmp/myinit
touch /tmp/myinit/in

log_to_result "Filling config of myinit daemon"
echo /bin/sleep 15 /tmp/myinit/in /tmp/myinit/out > /tmp/myinit/config
echo /bin/sleep 10 /tmp/myinit/in /tmp/myinit/out >> /tmp/myinit/config
echo /bin/sleep 20 /tmp/myinit/in /tmp/myinit/out >> /tmp/myinit/config

log_to_result "Compiling myinit daemon"
make myinit && log_to_result "myinit daemon compiled successfully!"
chmod 700 myinit

log_to_result "Starting myinit daemon"
./myinit /tmp/myinit/config && log_to_result "Started daemon. Processes launched: "

ps -ef | grep /bin/sleep | grep -v 'grep' >> result.txt

log_to_result "Killing second process"
pkill -f "/bin/sleep 10"

log_to_result "Waiting for 1 second"
sleep 1

log_to_result "Check running processes: "
ps -ef | grep /bin/sleep | grep -v 'grep' >> result.txt

log_to_result "Modifying config of myinit daemon"
echo /bin/sleep 50 /tmp/myinit/in /tmp/myinit/out > /tmp/myinit/config
sleep 7

log_to_result "Restarting myinit daemon with SIGHUP"
killall -SIGHUP myinit

log_to_result "Processes running after restart of myinit daemon: "
ps -ef | grep /bin/sleep | grep -v 'grep' >> result.txt

log_to_result "Reading logs of myinit daemon: "
cat /tmp/myinit.log >> result.txt

killall myinit
