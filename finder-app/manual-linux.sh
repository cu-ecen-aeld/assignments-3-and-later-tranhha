#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE mrproper #deep clean kernel build tree
	make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE defconfig #use default config 
	make -j4 ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE all #build kernel image
	make  ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE dtbs #build devicetree
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir rootfs && cd rootfs
mkdir home bin dev etc lib lib64 proc sys sbin tmp usr var 
mkdir usr/bin usr/sbin var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
	make distclean
	make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
	make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
	make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs

#Retrieve the program interpreter again
#awk -F': ' to split the line into fields where ': ' is a delimiter, then we only care the 2nd field which is path to interpreter
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
interpreter=$(${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter" | awk -F': ' '{print $2}' | tr -d '[]')

cp -a $SYSROOT/$interpreter ${OUT_DIR}/rootfs/lib #copy the program interpreter to rootfs/lib

cp -a ${SYSROOT}/lib/ld-linux-aarch64.so.1 ${OUT_DIR}/rootfs/lib
cp -a ${SYSROOT}/lib64/libm.so.6 ${OUT_DIR}/rootfs/lib64
cp -a ${SYSROOT}/lib64/libc.so.6 ${OUT_DIR}/rootfs/lib64
cp -a ${SYSROOT}/lib64/libresolv.so.2 ${OUT_DIR}/rootfs/lib64
# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
cd ~/assignments/assignment-1-tranhha/finder-app
make clean
make

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp writer* finder.sh autorun-qemu.sh conf/username.txt conf/assignment.txt ${OUT_DIR}/rootfs/home
# TODO: Chown the root directory
cd ${OUT_DIR}/rootfs
sudo chown -R root:root
# TODO: Create initramfs.cpio.gz
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUT_DIR}
gzip -f initramfs.cpio

