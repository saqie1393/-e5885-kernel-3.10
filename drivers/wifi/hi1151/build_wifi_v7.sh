#!/bin/sh

#WIFI_HOME=$(cd "$(dirname "$0")"; pwd)
#echo "WiFi WIFI_HOME=${WIFI_HOME}"
#WIFI_MAKEFILE_DIR=${WIFI_HOME}
#echo "WiFi MAKEFILE_DIR=${WIFI_MAKEFILE_DIR}"

#if [ 0 -eq $(echo x${WIFI_MAKEFILE_DIR} | grep -c 'modem\/system\/external') ];  then
#    PROJ_HOME=${WIFI_MAKEFILE_DIR}/../../../../..
#else
#    PROJ_HOME=${WIFI_MAKEFILE_DIR}/../../../../../../../
#fi
#PROJ_HOME=$(cd "$(dirname "${PROJ_HOME}/.")"; pwd)
#echo "WiFi PROJ_HOME=${PROJ_HOME}"

# echo "WiFi OBB_JOBS_1=${OBB_JOBS}"
# if [ -z "${OBB_JOBS}" ];  then
    # OBB_JOBS="-j4"
# else
	# OBB_JOBS=
# fi
# echo "WiFi OBB_JOBS_2=${OBB_JOBS}"


# export CFG_OS_ANDROID_PRODUCT_NAME=balong

# export ANDROID_KERN_DIR=${PROJ_HOME}/system/kernel
# echo "ANDROID_KERN_DIR=${ANDROID_KERN_DIR}"
# OUT_KERNEL=${ANDROID_KERN_DIR}/../out/target/product/${CFG_OS_ANDROID_PRODUCT_NAME}/obj/KERNEL_OBJ


# #export BOARD_TYPE=BOARD_ASIC
# export VERSION_TYPE=CHIP_BB_6920CS
# export PRODUCT_CFG_BUILD_TYPE=ALLY 

# export ARCH=arm
# export DHDARCH=arm
# export VERBOSE=1


# export LINUXDIR=${OUT_KERNEL}
# export SOCDIR=${PROJ_HOME}/modem/platform/hi6932/common/soc
# export HI6950_E5_CONFIG=${PROJ_HOME}/modem/config/product/hi6932_E5785Ch-22c/config
# export CONFIGDIR=${PROJ_HOME}/modem/config/product/hi6921_v711_e5/config/
# export INC_DRV_ACORE=${PROJ_HOME}/modem/include/drv/acore
# export BSPMODEMTIMER=${PROJ_HOME}/modem/include/drv/common
# export BSPOSMDIR=${PROJ_HOME}/modem/drv/common/include
# export INCLUDEDIR=${PROJ_HOME}/include/drv/

# echo "LINUXDIR=${LINUXDIR}"
# echo "SOCDIR=${SOCDIR}"
# echo "HI6950_E5_CONFIG=${HI6950_E5_CONFIG}"
# echo "CONFIGDIR=${CONFIGDIR}"
# echo "INC_DRV_ACORE=${INC_DRV_ACORE}"
# echo "BSPOSMDIR=${BSPOSMDIR}"
# echo "INCLUDEDIR=${INCLUDEDIR}"

# export CROSS_PATH=${PROJ_HOME}/system/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin
# echo "CROSS_PATH=${CROSS_PATH}"
# #export PATH=$PATH:/tools/linux/local-rh7.1/bin/:$CROSS_PATH
# export CROSS_COMPILE=${CROSS_PATH}/arm-linux-androideabi-


# export VERBOSE=1
# export LINUXVER=3.10
# #export CROSS_COMPILE=${PROJ_HOME}/system/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.7/bin/arm-linux-androideabi-
# #export ANDROID_KERN_DIR=${PROJ_HOME}/system/kernel
# export OUT_KERNEL=${ANDROID_KERN_DIR}/../out/target/product/${CFG_OS_ANDROID_PRODUCT_NAME}/obj/KERNEL_OBJ
# echo "OUT_KERNEL=${OUT_KERNEL}"

# #define default E5 config
# export PLATFORM_BOARD_TYPE=BALONG750
# export CONFIG_TARGET_PRODUCT=E5
# export COMPLIE_KO_IN_BALONGPLATFORM=y
# export DBG_ALG_ENABLE=y
# export CHIP_VERSION=ASIC
# #define default 722 platform config, later delete
# export COMPLIE_KO_PLATFORM=722platform
# echo "PLATFORM_BOARD_TYPE=${PLATFORM_BOARD_TYPE}"
# echo "COMPLIE_KO_PLATFORM=${COMPLIE_KO_PLATFORM}"

# #WIFI_OUT_SYSTEM_BIN=${OUT_KERNEL}/../../system/bin
# #echo "PATH="${PATH}
# echo "CROSS_COMPILE="${CROSS_COMPILE}
#echo "LINUXDIR="${LINUXDIR}

#cd ${WIFI_MAKEFILE_DIR}

# hisi provide Wi-Fi ko instead of source code
#make ko_clean

#make -s -C ${WIFI_MAKEFILE_DIR} ${OBB_JOBS} V=${VERBOSE} O=${OUT_KERNEL}
#make ko -s -C ${WIFI_MAKEFILE_DIR} ${OBB_JOBS} 2>&1 | tee log.txt |grep -E "warning|error|Error"

#make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}  KDIR=${LINUXDIR} -s -C ${CUR_PATH}/ ${WIFI_MAKE_TARGETS} ${OBB_JOBS} LINUXVER=${LINUXVER} CONFIG_BCMDHD=m CONFIG_BCM4356=y CONFIG_BCMDHD_PCIE=1 DRIVER_TYPE=m V=1 
#copy to out dir

#ATPV_OUT_DIR=${PROJ_HOME}/modem/atpv2/out/target/product/p722/system/bin

#echo "ATPV_OUT_DIR = ${ATPV_OUT_DIR}"
#echo "OBB_PRODUCT_NAME = ${OBB_PRODUCT_NAME}"

#cd ${ATPV_OUT_DIR}

#rm -rf ${ATPV_OUT_DIR}/wifi_hisi
#mkdir wifi_hisi

#cp -rf ${WIFI_HOME}/ko/*.ko ${ATPV_OUT_DIR}/wifi_hisi/
#cp -rf ${WIFI_HOME}/tool/*.sh ${ATPV_OUT_DIR}/wifi_hisi/
#cp -rf ${WIFI_HOME}/tool/hipriv ${ATPV_OUT_DIR}/
#cp -rf ${WIFI_HOME}/tool/hisi_wlan ${ATPV_OUT_DIR}/
#cp -rf ${WIFI_HOME}/tool/hisi_wlan_driver ${ATPV_OUT_DIR}/
#cp -rf ${WIFI_HOME}/tool/iwpriv ${ATPV_OUT_DIR}/
#cp -rf ${WIFI_HOME}/tool/libiw.so.29 ${ATPV_OUT_DIR}/../lib
#cp -rf ${WIFI_HOME}/product/${OBB_PRODUCT_NAME}/ini/* ${ATPV_OUT_DIR}/wifi_hisi/
#cp -rf ${WIFI_HOME}/product/${OBB_PRODUCT_NAME}/conf/* ${ATPV_OUT_DIR}/wifi_hisi/
