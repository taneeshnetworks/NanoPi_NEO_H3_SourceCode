#!/bin/sh

source send_cmd_pipe.sh

nr="0"

#get boot start flash type:0: nand, 1:card0, 2:emmc/tsd
BOOT_TYPE=-1
for parm in $(cat /proc/cmdline) ; do
	case $parm in
		boot_type=*)
			BOOT_TYPE=`echo $parm | awk -F\= '{print $2}'`
			;;
	esac
done

case $BOOT_TYPE in
	2)
		echo "boot_type = 2"
		nr="1"
		;;
	*)
		echo "boot_type = $BOOT_TYPE"
		nr="0"
		;;
esac

mmcblk="/dev/mmcblk$nr"
mmcp=$mmcblk

while true; do
    while true; do
        while true; do
            if [ -b "$mmcblk" ]; then
                sleep 1
                if [ -b "$mmcblk" ]; then
                    echo "card0 insert"
                    break
                fi
            else
                sleep 1
            fi
        done

        if [ ! -d "/tmp/extsd" ]; then
            mkdir /tmp/extsd
        fi

        mmcp=$mmcblk
        mount $mmcp /tmp/extsd
        if [ $? -ne 0 ]; then
            mmcp=$mmcblk"p1"
            mount $mmcp /tmp/extsd
            if [ $? -ne 0 ]; then
                SEND_CMD_PIPE_FAIL $3
                sleep 3
                continue 2
            fi
        fi

        break
    done

    capacity=`df -h | grep $mmcp | awk '{printf $2}'`
    echo "$mmcp: $capacity"
    
   # nandrw "/tmp/extsd/test.bin" "8" 
    
    if [ $? -ne 0 ]; then
        SEND_CMD_PIPE_FAIL $3
        exit 0
    fi

    umount /tmp/extsd

    SEND_CMD_PIPE_OK_EX $3 $capacity

    while true; do
        if [ -b "$mmcblk" ]; then
            sleep 1
        else
            echo "card0 remove"
            break
        fi
    done
done
