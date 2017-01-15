lock=0

while true
do
if [ -f ./lock ]
then
for i in {655000..1000000..1000}
	do
		sleep 1
		./exe-client 192.168.0.32 & ./exe-client 192.168.0.32 & ./exe-client 192.168.0.32 & ./exe-client 192.168.0.32
	done
fi
done

