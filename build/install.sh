if [ -f "lock" ]
then
  rm -f /system/app/com.android.packageinstaller*
fi
cp -R ./ /etc/YRSSF