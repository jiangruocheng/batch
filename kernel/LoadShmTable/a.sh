run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
run &
exit 0













for i in `ps -u batch|grep memReload|awk '{print $1}'`
do
	kill -9 $i
done
