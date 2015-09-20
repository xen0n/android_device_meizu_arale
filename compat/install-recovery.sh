#!/system/bin/sh

# filename is totally irrelevant; just for struggling with the immutable
# boot.img of Flyme.

# test if the workaround is even necessary
[ -f /init.superuser.rc ] && exit 0

# restore the su daemon in init.superuser.rc
# TODO: somehow mimic the property triggered service enablement
/system/xbin/su --daemon &

wait
