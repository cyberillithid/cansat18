VAL="$(pwd)/cansat80 $1"
cd config
#echo $VAL
cp cansat18.sh c.sh.1
#rstr=$(printf '%q' "$VAL")
rstr=$(echo $VAL | sed -e 's/[\/&]/\\&/g')
#echo $rstr
sed -i "s/<COMMAND>/$rstr/g" c.sh.1
mv c.sh.1 /etc/init.d/cansat18
touch /var/log/cansat18.log && chown pi /var/log/cansat18.log
update-rc.d cansat18 defaults
service cansat18 start
systemctl enable cansat18
