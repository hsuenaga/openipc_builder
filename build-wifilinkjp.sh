#!/bin/sh
PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/lib/wsl/lib:/snap/bin"
BUILDER="./builder.sh"

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

if [ -z $1 ]; then
	TARGET="ssc338q_fpv_runcam-wifilinkjp"
else
	TARGET=$1
fi
echo "execute ${BUILDER} ${OPT_U} ${TARGET}"
exec ${BUILDER} ${OPT_U} ${TARGET} > buildlog.txt 2>&1
#exec ./repack.sh ssc338q ssc338q_fpv_runcam-wifilink-nor wifilink wifilink
