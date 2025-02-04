#!/bin/sh

md5sum_result="equal"

SDMMC_DEV="/dev/mmcblk0"
SYS_CONF_FILE_PATH="/etc"
SYS_TMP="/tmp"
SYS_USR_BIN_PATH="/usr/bin"
SDMMC_PATH="/mnt/mmcblk0p1"

WFB_FILE="$SYS_CONF_FILE_PATH/wfb.conf"
MAJESTIC_FILE="$SYS_CONF_FILE_PATH/majestic.yaml"
SYS_OPENIPC_CONF_INI_FILE="$SYS_CONF_FILE_PATH/user.ini"
DATALINK_CONF_FILE="$SYS_CONF_FILE_PATH/datalink.conf"

OPENIPC_CONF_INI_FILE="$SDMMC_PATH/user.ini"

INI_FILE="$OPENIPC_CONF_INI_FILE"


DRONE_KEY_PATH="$SYS_CONF_FILE_PATH/drone.key"
GS_KEY_PATH="$SYS_CONF_FILE_PATH/gs.key"

SD_DRONE_KEY_PATH="$SDMMC_PATH/drone.key"
SD_GS_KEY_PATH="$SDMMC_PATH/gs.key"

KERNEL_FILE_NAME="WiFiLink-part0.bin"
ROOTFS_FILE_NAME="WiFiLink-part1.bin"

SD_CARD_UPGRADE_KERNEL_FILE_PATH="$SDMMC_PATH/$KERNEL_FILE_NAME"
SD_CARD_UPGRADE_ROOTFS_FILE_PATH="$SDMMC_PATH/$ROOTFS_FILE_NAME"

MSP_OSD_FILE_PATH="$SYS_USR_BIN_PATH/msposd"

MSP_OSD_FILE_ETC_PATH="$SDMMC_PATH/msposd"
MSP_OSD_FONT_FILE_ETC_PATH="$SDMMC_PATH/font.png"
MSP_OSD_FONT_HD_FILE_ETC_PATH="$SDMMC_PATH/font_hd.png"

exit_count=1

ini_get() {
    #awk -F '=' "/^$KEY/ {print \$2; exit}" KEY=$KEY $INI_FILE
    awk -F '=' "/^$KEY/ {print \$2; exit}" KEY=$KEY $1
}

majestic_yaml_get(){
	value=$(yaml-cli -i $MAJESTIC_FILE -g .$1.$2)
}

majestic_yaml_video0_size_get(){
	yaml-cli -i $MAJESTIC_FILE -g .video0.size
}

majestic_yaml_video0_fps_get(){
	yaml-cli -i $MAJESTIC_FILE -g .video0.fps
}

majestic_yaml_set(){
	yaml-cli -i $MAJESTIC_FILE -s .$1.$2 $3
}

update_kernel_or_rootfs() {
    if [ -e $SD_CARD_UPGRADE_KERNEL_FILE_PATH ]; then
        cp $SD_CARD_UPGRADE_KERNEL_FILE_PATH $SYS_TMP
        sleep 1
        rm $SD_CARD_UPGRADE_KERNEL_FILE_PATH
        md5sum_result="update"
    fi

    if [ -e $SD_CARD_UPGRADE_ROOTFS_FILE_PATH ]; then
        cp $SD_CARD_UPGRADE_ROOTFS_FILE_PATH $SYS_TMP
        sleep 2
        rm $SD_CARD_UPGRADE_ROOTFS_FILE_PATH
        md5sum_result="update"
    fi
    
    if [ "${md5sum_result}" == "update" ]; then
        echo "kernel or rootfs update."
        sleep 1
        /etc/upgrade_led.sh &
        sysupgrade --kernel=/tmp/$KERNEL_FILE_NAME --rootfs=/tmp/$ROOTFS_FILE_NAME -n
        exit 0
    fi
}

md5sum_gs_key()
{
    if [ -e $SD_GS_KEY_PATH ]; then
        #echo "$SD_GS_KEY_PATH exists."
        GS_KEY_MD5SUM=$(md5sum $DRONE_KEY_PATH | awk 'NR==1{print $1}')
        SD_GS_KEY_MD5SUM=$(md5sum $SD_GS_KEY_PATH | awk 'NR==1{print $1}')
        if [ "${GS_KEY_MD5SUM}" != "${SD_GS_KEY_MD5SUM}" ]; then
            cp $DRONE_KEY_PATH $SD_GS_KEY_PATH
        fi
    else
        #echo "$SD_GS_KEY_PATH not exists."
        cp $DRONE_KEY_PATH $SD_GS_KEY_PATH
    fi
}

cp_drone_key_to_sd_card()
{
    if [ ! -e $SD_GS_KEY_PATH ]; then
    echo "cp drone key to sd card."
    cp $DRONE_KEY_PATH $SD_GS_KEY_PATH
    fi
}

channel_check_set()
{
    KEY="channel"
    ini_value=$(ini_get "$OPENIPC_CONF_INI_FILE" "$KEY")
    value=$(ini_get "$WFB_FILE" "$KEY")
    #echo "$KEY ini: $ini_value"
    #echo "$KEY wfb: $value"

    if [ "${ini_value}" != "${value}" ]; then
        md5sum_result="unequal"
        sed -i "s/${KEY}=.*$/${KEY}=${ini_value}/g" "$WFB_FILE"
        echo "set $KEY value $ini_value"
    fi
}

driver_txpower_override_check_set()
{
    KEY="driver_txpower_override"

    ini_value=$(ini_get "$OPENIPC_CONF_INI_FILE" "$KEY")
    value=$(ini_get "$WFB_FILE" "$KEY")
    #echo "$KEY ini: $ini_value"
    #echo "$KEY wfb: $value"

    if [ "${ini_value}" != "${value}" ]; then
        md5sum_result="unequal"
        sed -i "s/${KEY}=.*$/${KEY}=${ini_value}/g" "$WFB_FILE"
        echo "set $KEY value $ini_value"
    fi
}

msp_osd_file_check()
{
    if [ ! -e $MSP_OSD_FILE_PATH ]; then
    echo "copy msp osd related files to /usr/bin."
    cp $MSP_OSD_FILE_ETC_PATH $SYS_USR_BIN_PATH
    cp $MSP_OSD_FONT_FILE_ETC_PATH $SYS_USR_BIN_PATH
    cp $MSP_OSD_FONT_HD_FILE_ETC_PATH $SYS_USR_BIN_PATH
    chmod 0755 $SYS_USR_BIN_PATH/msposd
    sleep 1
    rm $MSP_OSD_FILE_ETC_PATH
    rm $MSP_OSD_FONT_FILE_ETC_PATH 
    rm $MSP_OSD_FONT_HD_FILE_ETC_PATH
    md5sum_result="unequal"
    fi
}

osd_protocol_check_set()
{
    KEY="protocol"
    ini_value=$(ini_get "$OPENIPC_CONF_INI_FILE" "$KEY")

    if [ "${ini_value}" == "mavlink" ]; then
        ini_value="true"
    elif [ "${ini_value}" == "msp" ]; then
        ini_value="false"
    else
        echo "protocol val err."
        exit 0
    fi

    KEY="telemetry"
    value=$(ini_get "$DATALINK_CONF_FILE" "$KEY")

    if [ "${ini_value}" != "${value}" ]; then
            sed -i "s/${KEY}=.*$/${KEY}=${ini_value}/g" "$DATALINK_CONF_FILE"
            md5sum_result="unequal"
    fi

}

mps_osd_start()
{
    if [ -e $MSP_OSD_FILE_PATH ]; then
        KEY="telemetry"
        value=$(ini_get "$DATALINK_CONF_FILE" "$KEY")
        if [ "${value}" == "false" ]; then
            msposd --master /dev/ttyS2 --baudrate 115200 --channels 8 --out 127.0.0.1:14555 -osd -r 20 --ahi 0 --wait 5 --persist 50 -v &
        fi
    fi
}

mcs_index_check_set()
{
    KEY="mcs_index"
    ini_value=$(ini_get "$OPENIPC_CONF_INI_FILE" "$KEY")
    value=$(ini_get "$WFB_FILE" "$KEY")
    echo "$KEY ini: $ini_value"
    echo "$KEY wfb: $value"

    if [ "${ini_value}" != "${value}" ]; then
        md5sum_result="unequal"
        sed -i "s/${KEY}=.*$/${KEY}=${ini_value}/g" "$WFB_FILE"
        echo "set $KEY value $ini_value"
    fi
}

majestic_yaml_image_check_set()
{
    if [ "$1" == "contrast" ] || [ "$1" == "hue" ] || [ "$1" == "saturation" ] || [ "$1" == "luminance" ] || [ "$1" == "mirror" ] || [ "$1" == "flip" ] || [ "$1" == "rotate" ]; then
        KEY=$1
        ini_value=$(ini_get "$OPENIPC_CONF_INI_FILE" "$KEY")
        majestic_yaml_get "image" "$1"

        #echo "$1 ini:$ini_value"
        #echo "$1 maj:$value"

        if [ "${ini_value}" != "${value}" ]; then
            if [ "$1" == "contrast" ] || [ "$1" == "hue" ] || [ "$1" == "saturation" ] || [ "$1" == "luminance" ]; then
                if [[ "$ini_value" =~ ^-?[0-9]+$ ]]; then
                    num_int=$(expr $ini_value)
                    if [ $num_int -ge 0 ] && [ $num_int -le 100 ]; then
                        md5sum_result="unequal"
                        echo "$ini_value as an integer is in the range 0 to 100."
                        majestic_yaml_set "image" $1 $ini_value
                    else
                        echo "$ini_value as an integer is NOT in the range 0 to 100."
                        return 1
                    fi
                else
                    echo "Error: '$ini_value' is not a valid integer."
                    return 1
                fi
            fi

            if [ "$1" == "mirror" ] || [ "$1" == "flip" ]; then
                if [ "${ini_value}" == "true" ] || [ "${ini_value}" == "false" ]; then
                    md5sum_result="unequal"
                    majestic_yaml_set "image" $1 $ini_value
                else
                    echo "set $1 err"
                    exit 0
                fi
            fi

            if [ "$1" == "rotate" ]; then
                if [ "${ini_value}" == "0" ] || [ "${ini_value}" == "90" ] || [ "${ini_value}" == "180" ] || [ "${ini_value}" == "270" ]; then
                    md5sum_result="unequal"
                    majestic_yaml_set "image" $1 $ini_value
                else
                    echo "set $1 err"
                    exit 0
                fi
            fi
        
            echo "set maj value $value to $ini_value"
        fi
    else
        echo "$1 Not the correct parameter"
        return 1
    fi
}

majestic_yaml_video0_check_set()
{
    #if [ "$1" == "codec" ] || [ "$1" == "size" ] || [ "$1" == "fps" ] || [ "$1" == "bitrate" ]; then
    if [ "$1" == "codec" ] || [ "$1" == "bitrate" ]; then
        KEY=$1
        if [ "$KEY" == "size" ]; then
            KEY="ResolutionRatio"
        fi
        ini_value=$(ini_get "$OPENIPC_CONF_INI_FILE" "$KEY")
        majestic_yaml_get "video0" "$1"

        #echo "$1 ini:$ini_value"
        #echo "$1 maj:$value"

        if [ "${ini_value}" != "${value}" ]; then
            md5sum_result="unequal"
            #codec
            if [ "$1" == "codec" ]; then
                if [ "${ini_value}" == "h264" ] || [ "${ini_value}" == "h265" ]; then
                    majestic_yaml_set "video0" $1 $ini_value
                else
                    echo "set codec err"
                    exit 0
                fi
            fi

            #size
            if [ "$1" == "size" ]; then
                if [ "${ini_value}" == "1920x1080" ] || [ "${ini_value}" == "1280x720" ]; then
                    majestic_yaml_set "video0" $1 $ini_value
                else
                    echo "set size err"
                    exit 0
                fi
            fi

            #fps
            if [ "$1" == "fps" ]; then
                if [ "${ini_value}" == "60" ] || [ "${ini_value}" == "90" ] || [ "${ini_value}" == "120" ]; then
                    majestic_yaml_set "video0" $1 $ini_value
                else
                    echo "set fps err"
                    exit 0
                fi
            fi

            if [ "$1" == "bitrate" ]; then
                if [ "${ini_value}" == "4096" ] || [ "${ini_value}" == "5120" ] || [ "${ini_value}" == "6144" ] || [ "${ini_value}" == "7168" ] || [ "${ini_value}" == "8192" ]; then
                    majestic_yaml_set "video0" $1 $ini_value
                else
                    echo "set $1 err"
                    exit 0
                fi
            fi
            echo "set maj value $value to $ini_value"
        fi
    elif [ "$1" == "VideoSize" ]; then
        KEY=$1
        ini_value=$(ini_get "$OPENIPC_CONF_INI_FILE" "$KEY")
        maj_video0_size=$(majestic_yaml_video0_size_get)
        maj_video0_fps=$(majestic_yaml_video0_fps_get)

        #echo "$1 ini:$ini_value"
        #echo "$1 maj_size:$maj_video0_size"
        #echo "$1 maj_fps:$maj_video0_fps"

        if [ "${ini_value}" == "720p60fps" ]; then
            if [ "1280x720" != "${maj_video0_size}" ] || [ "60" != "${maj_video0_fps}" ]; then
                md5sum_result="unequal"
                majestic_yaml_set "video0" "size" "1280x720"
                majestic_yaml_set "video0" "fps" "60"
            fi
        elif [ "${ini_value}" == "720p90fps" ]; then
            if [ "1280x720" != "${maj_video0_size}" ] || [ "90" != "${maj_video0_fps}" ]; then
                md5sum_result="unequal"
                majestic_yaml_set "video0" "size" "1280x720"
                majestic_yaml_set "video0" "fps" "90"
            fi
        elif [ "${ini_value}" == "720p120fps" ]; then
            if [ "1280x720" != "${maj_video0_size}" ] || [ "120" != "${maj_video0_fps}" ]; then
                md5sum_result="unequal"
                majestic_yaml_set "video0" "size" "1280x720"
                majestic_yaml_set "video0" "fps" "120"
            fi
        elif [ "${ini_value}" == "1080p60fps" ]; then
            if [ "1920x1080" != "${maj_video0_size}" ] || [ "60" != "${maj_video0_fps}" ]; then
                md5sum_result="unequal"
                majestic_yaml_set "video0" "size" "1920x1080"
                majestic_yaml_set "video0" "fps" "60"
            fi
        elif [ "${ini_value}" == "1080p90fps" ]; then
            if [ "1920x1080" != "${maj_video0_size}" ] || [ "90" != "${maj_video0_fps}" ]; then
                md5sum_result="unequal"
                majestic_yaml_set "video0" "size" "1920x1080"
                majestic_yaml_set "video0" "fps" "90"
            fi   
        else
            echo "set video size err"
            exit 0
        fi
    else
        echo "$1 Not the correct parameter"
        return 0
    fi
}

majestic_yaml_records_check_set()
{
    KEY="records"
    ini_value=$(ini_get "$OPENIPC_CONF_INI_FILE" "$KEY")
    majestic_yaml_get "records" "enabled"

    #echo "ini:$ini_value"
    #echo "maj:$value"

    if [ "${ini_value}" != "${value}" ]; then
        if [ "${ini_value}" == "true" ] || [ "${ini_value}" == "false" ]; then
            majestic_yaml_set "records" "enabled" "$ini_value"
            md5sum_result="unequal"
        else
            echo "Error opening video recording"
            exit 0
        fi
    fi

    majestic_yaml_get "records" "enabled"
    if [ "${value}" == "true" ]; then
        #random_number=$(shuf -i 1000-9999 -n 1)
        #majestic_yaml_set "records" "path" "/mnt/mmcblk0p1/%F$random_number"
        filename="/etc/recnum"
        read -r number < "$filename"
        majestic_yaml_set "records" "path" "/mnt/mmcblk0p1/%F-$number"
        new_number=$((number + 1))
        #echo "new_number: $new_number"
        echo "$new_number" > "/etc/recnum"
    fi
}

profile_settings()
{
    if [ -e $OPENIPC_CONF_INI_FILE ]; then

        channel_check_set
        driver_txpower_override_check_set

        majestic_yaml_video0_check_set "codec"
        majestic_yaml_video0_check_set "VideoSize"
        #majestic_yaml_video0_check_set "size"
        #majestic_yaml_video0_check_set "fps"
        majestic_yaml_video0_check_set "bitrate"
        
        majestic_yaml_image_check_set "mirror"
        majestic_yaml_image_check_set "flip"
        majestic_yaml_image_check_set "rotate"
        majestic_yaml_image_check_set "contrast"
        majestic_yaml_image_check_set "hue"
        majestic_yaml_image_check_set "saturation"
        majestic_yaml_image_check_set "luminance"

        majestic_yaml_records_check_set

        osd_protocol_check_set
    else
        cp $SYS_OPENIPC_CONF_INI_FILE $SDMMC_PATH
        chmod 777 $OPENIPC_CONF_INI_FILE
    fi

    #echo "$md5sum_result"
    if [ "${md5sum_result}" == "unequal" ]; then
        echo "reset dev ..."
        reboot
    fi
    
    exit 0
}

mps_osd_start

while true; do
    if [ -e $SDMMC_DEV ] && [ -e $SDMMC_PATH ]; then
        echo "Device $SDMMC_PATH exists."
        update_kernel_or_rootfs
        cp_drone_key_to_sd_card
        profile_settings
        exit 0
    else
        echo "Device $SDMMC_DEV does not exist."
        if [ $exit_count -eq 3 ]; then
            exit 0
        fi
        let "exit_count++"
        sleep 1
    fi
done  
