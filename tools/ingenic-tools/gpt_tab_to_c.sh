#!/bin/bash

file=$1
# 文件的形式如下
# 主要是把这个文件的分区表转换c语言的数组
# property:
#     disk_size = 4096m
#     gpt_header_lba = 512
#     custom_signature = 0

# partition:
# 	#name     =  start,   size, fstype
# 	xboot     =     0m,     1m,
# 	kernel    =     1m,     8m, EMPTY
# 	rootfs    =     9m,    100m, LINUX_FS
# 	userdata  =   109m,    3987m, LINUX_FS

#fstype could be: LINUX_FS, FAT_FS, EMPTY

# 获得字符串中的第几个word
get_word()
{
    local str="$1"
    local n=$2
    local i=0

    for word in $str
    do
        if [ $i = $n ]; then
            echo $word
            return 0
        fi
        let i=i+1
    done

    return 1
}

# 删除最右边的字符，如果存在的话
delete_r_char()
{
    local str="$1"
    local c=$2

    if [ "${str:0-1}" = "$c" ]; then
        str=${str%%$c}
        echo "$str"
        return 0
    fi

    echo "$str"
    return 1
}

# 删除最右边的字符，如果存在的话
delete_r_char2()
{
    local str="$1"
    local c1=$2
    local c2=$3

    if [ "${str:0-1}" = "$c1" ]; then
        str=${str%%$c1}
        echo "$str"
        return 0
    fi

    if [ "${str:0-1}" = "$c2" ]; then
        str=${str%%$c2}
        echo "$str"
        return 0
    fi

    echo "$str"
    return 1
}

parse_size()
{
    local str="$1"

    str=`delete_r_char2 $str b B`
    local str_g=`delete_r_char2 $str g G`
    local str_m=`delete_r_char2 $str m M`
    local str_k=`delete_r_char2 $str k K`

    if [ "$str_g" = "" ] || [ "$str_m" = "" ] || [ "$str_k" = "" ]; then
        echo "partition tab: skip bad format: $line" 1>&2
        return 1
    fi

    local ret=$str
    if [ "$str" != "$str_g" ]; then ret=$str_g"*GB_SECTOR"; fi
    if [ "$str" != "$str_m" ]; then ret=$str_m"*MB_SECTOR"; fi
    if [ "$str" != "$str_k" ]; then ret=$str_k"*KB_SECTOR"; fi

    echo $ret
    return 0
}

status="find-partition-lable"


echo "/*"
echo " * auto generate by"
echo " *   tool: $0"
echo " *   file: $file"
echo " * do not modify this file"
echo " */"
echo ""
echo "struct mmc_partition_data {"
echo "    const char *name;"
echo "    unsigned int offset_sector;"
echo "    unsigned int size_sector;"
echo "};"
echo ""
echo "#define SECTOR_SIZE 512"
echo "#define GB_SECTOR (1024*1024*1024/SECTOR_SIZE)"
echo "#define MB_SECTOR (1024*1024/SECTOR_SIZE)"
echo "#define KB_SECTOR (1024/SECTOR_SIZE)"
echo ""
echo "static struct mmc_partition_data partition_tab[] = {"

while read line;
do
    str=`get_word "$line" 0`

    if [ "$str" = "partition:" ]; then
        status="parse-partitions"
        continue
    fi

    if [ "$status" != "parse-partitions" ]; then
        continue
    fi

    # 检测到注释
    if [ "${str:0:1}" = "#" ]; then
        continue;
    fi

    # 检测到新的lable，则退出,形如 'lable:'
    if [ "${str:0-1}" = ":" ]; then
        break;
    fi

    # 检测 = 号
    str1=`get_word "$line" 1`
    if [ "$str1" != "=" ]; then
        continue
    fi

    # start 偏移
    str2=`get_word "$line" 2`

    # 检测逗号是否连着start写, 形如 1m,
    has_q=0
    str2=`delete_r_char $str2 ,`
    if [ "$?" = "0" ]; then
        has_q=1
    fi

    if [ "$str2" = "" ]; then
        echo "partition tab: skip bad format: $line" 1>&2
        continue;
    fi

    str2=`parse_size $str2`
    if [ "$?" != 0 ]; then
        echo "partition tab: skip bad format: $line" 1>&2
        continue
    fi

    str3=`get_word "$line" 3`
    if [ "$has_q" = "0" ]; then
        if [ "${str3:0:1}" != "," ]; then
            echo "partition tab: skip bad format: $line" 1>&2
            continue
        fi

        if [ "$str3" = "," ]; then
            str3=`get_word "$line" 4`
            if [ "$str3" = "" ]; then
                echo "partition tab: skip bad format: $line" 1>&2
                continue;
            fi
        else
            str3=${str3##,}
        fi
    fi

    str3=`delete_r_char $str3 ,`
    if [ "$str3" = "" ]; then
        echo "partition tab: skip bad format: $line" 1>&2
        continue;
    fi

    str3=`parse_size $str3`
    if [ "$?" != 0 ]; then
        echo "partition tab: skip bad format: $line" 1>&2
        continue
    fi

    partition_name=$str
    partition_start=$str2
    partition_size=$str3
    echo "    { \"$partition_name\", $partition_start, $partition_size },"

done < "$file"

echo "};"
