#!/system/bin/sh
echo 536870912 > /sys/block/zram0/disksize
/system/xbin/mkswap /dev/block/zram0
/system/xbin/swapon /dev/block/zram0
