#!/bin/sh

BUILD_DIR="/home/hsuenaga/Projects/openipc_builder/openipc/output/build/linux-custom"
TARGET=""
CLEANBUILD=""
VERBOSE=""
while getopts cC:v OPT; do
	case $OPT in
		c)
			CLEANBUILD="clean"
			;;
		C)
			BUILD_DIR=$OPTARG
			;;
		v)
			VERBOSE="quiet= V=5"
			;;
		*)
			;;
	esac
done
shift $((OPTIND - 1))

if [ "X$1" != "X" ]; then
	TARGET=$1
fi

export GIT_DIR=.
export PATH="/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/bin:/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/sbin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/lib/wsl/lib:/snap/bin"
export PKG_CONFIG="/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/bin/pkg-config"
export PKG_CONFIG_SYSROOT_DIR="/"
export PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1
export PKG_CONFIG_ALLOW_SYSTEM_LIBS=1
export PKG_CONFIG_LIBDIR="/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/lib/pkgconfig:/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/share/pkgconfig"
export BR_BINARIES_DIR=/home/hsuenaga/Projects/openipc_builder/openipc/output/images
export KCFLAGS=-Wno-attribute-alias

export HOSTCC="/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/bin/ccache /usr/bin/gcc -O2 -isystem /home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/include -L/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/lib -Wl,-rpath,/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/lib"
export ARCH=arm
export INSTALL_MOD_PATH=/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/target
export CROSS_COMPILE="/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/bin/arm-openipc-linux-gnueabihf-"
export WERROR=0
export REGENERATE_PARSERS=1
export DEPMOD=/home/hsuenaga/Projects/openipc_builder/openipc/output/per-package/linux/host/sbin/depmod
export INSTALL_MOD_STRIP=1

if [ "X$CLEANBUILD" != "X" ]; then
	/usr/bin/make -C $BUILD_DIR $VERBOSE $CLEANBUILD
fi

if [ "X$TARGET" != "X" ]; then
	/usr/bin/make -C $BUILD_DIR $VERBOSE $TARGET
else
	/usr/bin/make -C $BUILD_DIR $VERBOSE all && \
	/usr/bin/make -C $BUILD_DIR $VERBOSE uImage && \
	/usr/bin/make -C $BUILD_DIR $VERBOSE modules_install
fi
