#!/bin/sh

set -e

wheel_gid=1
tty_gid=2
phys_gid=3
audio_gid=4
window_uid=13
window_gid=13

script_path=$(cd -P -- "$(dirname -- "$0")" && pwd -P)

DISK_PATH=Target/disk
SYSROOT=Sysroots/Target
SERENITY_ROOT=$1

die() {
    echo "die: $*"
    exit 1
}

if [ "$(id -u)" != 0 ]; then
    die "this script needs to run as root"
fi

umask 0022

printf "creating initial filesystem structure... "
for dir in bin etc proc mnt tmp; do
    mkdir -p ${DISK_PATH}/$dir
done
chmod 1777 ${DISK_PATH}/tmp
echo "done"

printf "setting up device nodes... "
mkdir -p ${DISK_PATH}/dev
mkdir -p ${DISK_PATH}/dev/pts
mknod ${DISK_PATH}/dev/fb0 b 29 0
chmod 660 ${DISK_PATH}/dev/fb0
chown 0:$phys_gid ${DISK_PATH}/dev/fb0
mknod ${DISK_PATH}/dev/tty0 c 4 0
mknod ${DISK_PATH}/dev/tty1 c 4 1
mknod ${DISK_PATH}/dev/tty2 c 4 2
mknod ${DISK_PATH}/dev/tty3 c 4 3
mknod ${DISK_PATH}/dev/ttyS0 c 4 64
mknod ${DISK_PATH}/dev/ttyS1 c 4 65
mknod ${DISK_PATH}/dev/ttyS2 c 4 66
mknod ${DISK_PATH}/dev/ttyS3 c 4 67
for tty in 0 1 2 3 S0 S1 S2 S3; do
    chmod 620 ${DISK_PATH}/dev/tty$tty
    chown 0:$tty_gid ${DISK_PATH}/dev/tty$tty
done
mknod ${DISK_PATH}/dev/random c 1 8
mknod ${DISK_PATH}/dev/null c 1 3
mknod ${DISK_PATH}/dev/zero c 1 5
mknod ${DISK_PATH}/dev/full c 1 7
mknod ${DISK_PATH}/dev/debuglog c 1 18
# random, is failing (randomly) on fuse-ext2 on macos :)
chmod 666 ${DISK_PATH}/dev/random || true
chmod 666 ${DISK_PATH}/dev/null
chmod 666 ${DISK_PATH}/dev/zero
chmod 666 ${DISK_PATH}/dev/full
chmod 666 ${DISK_PATH}/dev/debuglog
mknod ${DISK_PATH}/dev/keyboard c 85 1
chmod 440 ${DISK_PATH}/dev/keyboard
chown 0:$phys_gid ${DISK_PATH}/dev/keyboard
mknod ${DISK_PATH}/dev/mouse c 10 1
chmod 440 ${DISK_PATH}/dev/mouse
chown 0:$phys_gid ${DISK_PATH}/dev/mouse
mknod ${DISK_PATH}/dev/audio c 42 42
chmod 220 ${DISK_PATH}/dev/audio
chown 0:$audio_gid ${DISK_PATH}/dev/audio
mknod ${DISK_PATH}/dev/ptmx c 5 2
chmod 666 ${DISK_PATH}/dev/ptmx
mknod ${DISK_PATH}/dev/hda b 3 0
mknod ${DISK_PATH}/dev/hdb b 3 1
mknod ${DISK_PATH}/dev/hdc b 4 0
mknod ${DISK_PATH}/dev/hdd b 4 1
for hd in a b c d; do
    chmod 600 ${DISK_PATH}/dev/hd$hd
done

ln -s /proc/self/fd/0 ${DISK_PATH}/dev/stdin
ln -s /proc/self/fd/1 ${DISK_PATH}/dev/stdout
ln -s /proc/self/fd/2 ${DISK_PATH}/dev/stderr
echo "done"

printf "installing base system... "
cp -R ${SERENITY_ROOT}/Base/* ${DISK_PATH}/
cp -R ${SYSROOT}/* ${DISK_PATH}/

# create kernel map
tmp=$(mktemp)
nm -n ${SYSROOT}/boot/kernel | awk '{ if ($2 != "a") print; }' | uniq > $tmp
printf "%08x\n" $(wc -l $tmp | cut -f1 -d' ') > ${DISK_PATH}/res/kernel.map
cat $tmp >> ${DISK_PATH}/res/kernel.map
rm -f $tmp

chmod 400 ${DISK_PATH}/res/kernel.map

chmod 660 ${DISK_PATH}/etc/WindowServer/WindowServer.ini
chown $window_uid:$window_gid ${DISK_PATH}/etc/WindowServer/WindowServer.ini

echo "done"

printf "installing users... "
mkdir -p ${DISK_PATH}/home/anon
mkdir -p ${DISK_PATH}/home/nona

chmod 700 ${DISK_PATH}/home/anon
chmod 700 ${DISK_PATH}/home/nona
chown -R 100:100 ${DISK_PATH}/home/anon
chown -R 200:200 ${DISK_PATH}/home/nona
echo "done"

printf "installing userland... "

# if [ "$(uname -s)" = "Darwin" ]; then
#     find ../Userland/ -type f -perm +111 -exec cp {} ${SYSROOT}/bin/ \;
# elif [ "$(uname -s)" = "OpenBSD" ]; then
#     find ../Userland/ -type f -perm -555 -exec cp {} ${SYSROOT}/bin/ \;
# else
#     find ../Userland/ -type f -executable -exec cp {} ${SYSROOT}/bin/ \;
# fi
chown 0:$wheel_gid ${DISK_PATH}/bin/su
chown 0:$phys_gid ${DISK_PATH}/bin/shutdown
chown 0:$phys_gid ${DISK_PATH}/bin/reboot
chmod 4750 ${DISK_PATH}/bin/su
chmod 4755 ${DISK_PATH}/bin/ping
chmod 4750 ${DISK_PATH}/bin/reboot
chmod 4750 ${DISK_PATH}/bin/shutdown
echo "done"

# Run local sync script, if it exists
if [ -f sync-local.sh ]; then
    sh sync-local.sh
fi
