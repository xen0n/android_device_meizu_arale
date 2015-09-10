#!/system/bin/sh

# filename is totally irrelevant; just for struggling with the immutable
# boot.img of Flyme.

# restore the su daemon in init.superuser.rc
# TODO: somehow mimic the property triggered service enablement
/system/xbin/su --daemon &

wait
