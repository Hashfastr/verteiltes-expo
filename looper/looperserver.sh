lock=0

while true
do
if [ -f ./lock ]
then
for i in {655000..1000000..1000}
	do
		./exe $i
	done
rm ./lock
curl 'https://api.simplepush.io/send/w4TWm4/Wow/So easy'
fi
done

