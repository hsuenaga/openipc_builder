#!/bin/sh
OPT_U=""
OPT_L="-l"
while getopts u OPT; do
	case $OPT in
		u)
			OPT_U="-u"
			;;
		*)
			exit 0
			;;
	esac
done
shift $((OPTIND - 1))

PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/lib/wsl/lib:/snap/bin"
BUILDER="./builder.sh"
BUILDLOG="./buildlog.txt"
KERNEL="./archive/latest/uImage.ssc338q"
ROOTFS="./archive/latest/rootfs.squashfs.ssc338q"

if [ -z $1 ]; then
	TARGET="ssc338q_fpv_runcam-wifilinkjp"
else
	TARGET=$1
fi

echo "execute ${BUILDER} ${OPT_U} ${TARGET}"
echo "buildlog is ${BUILDLOG}"
${BUILDER} ${OPT_U} ${OPT_L} "${TARGET}" > "${BUILDLOG}" 2>&1
if [ -f "${KERNEL}" -a -f "${ROOTFS}" ]; then
	echo "Copy firmware image."
	cp "${KERNEL}" "WiFiLink-part0.bin"
	cp "${ROOTFS}" "WiFiLink-part1.bin"
fi
echo "Done."
