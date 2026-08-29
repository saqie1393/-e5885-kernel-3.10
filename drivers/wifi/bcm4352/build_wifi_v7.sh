#!/bin/sh

WIFI_HOME=$(cd "$(dirname "$0")"; pwd)
echo "WiFi WIFI_HOME=${WIFI_HOME}"

if [ 0 -eq $(echo ${WIFI_HOME} | grep -c 'modem\/system\/external') ];  then
    PROJ_HOME=${WIFI_HOME}/../../../../..
else
    PROJ_HOME=${WIFI_HOME}/../../../../../
fi
PROJ_HOME=$(cd "$(dirname "${PROJ_HOME}/.")"; pwd)
echo "PROJ_HOME=${PROJ_HOME}"

#copy to out dir
#MBB修改，[注意]:平台切换后，p722的参数必须修改
ATPV2_OUT_DIR=${PROJ_HOME}/modem/atpv2/out/target/product/p722/system/bin
echo "atpv2 out dir=${ATPV2_OUT_DIR}"

WIFI_KO_DIR=${PROJ_HOME}/modem/atpv2/vendor/atp/driver/component/wlan/bcm/bcm4352/driver/p722/C60
echo "wifi ko dir=${WIFI_KO_DIR}"

if [ -e "${WIFI_KO_DIR}/wl.ko" ];then
	cp -rf ${WIFI_KO_DIR}/wl.ko ${ATPV2_OUT_DIR}/bcm4352/driver/wl.ko
fi

exit 0
