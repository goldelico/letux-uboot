#!/usr/bin/env python3
import struct
import argparse

'''
介绍：本文件为uboot升级进行重新打包uboot内容
uboot在烧录的过程烧录工具进行写入了一些内容
该内容包含DDRID、secureid和partition，通过烧录工具将
烧录到板子内的uboot进行读取后进行与SDK编译出的uboot进行拼接
组成好的uboot就可以进行OTA升级启动
'''


'''
#define NOR_PART_NUM					10
struct nor_partition {
	char name[32];
	uint32_t size;
	uint32_t offset;
	uint32_t mask_flags;//bit 0-1 mask the partiton RW mode, 0:RW  1:W  2:R
	uint32_t manager_mode;
};
'''

def main():
    # 创建解析器
    parser = argparse.ArgumentParser(description="使用实例:./merger_uboot.py -p uboot.template.bin -i uboot -o uboot-ok")

    # 添加参数
    parser.add_argument("--param", "-p", help="输入文件也就是读取后的uboot内容这里成为 uboot.template.bin", type=str, required=True)
    parser.add_argument("--input", "-i", help="SDK编译后的uboot文件", type=str, required=True)
    parser.add_argument("--output", "-o", help="repack后的uboot文件", type=str, default="uboot.bin")

    # 解析参数
    args = parser.parse_args()

    # 打印参数值
    print(f"输入文件路径: {args.input}")
    print(f"输出文件路径: {args.output}")
    inFile = args.input
    paramFile = args.param
    outFile = args.output


    ddrID=0
    secureID=0
    partition=0
    with open(paramFile, "rb") as file:
        file.seek(128)
        ddrID = file.read(4)  # 读取4字节数据
        file.seek(512)
        secureID = file.read(2)
        file.seek(0x5800)
        partition = file.read(1024)
    with open(inFile, "rb") as iFile , open(outFile, "wb") as oFile:
        data = iFile.read(128)
        oFile.write(data)
        oFile.write(ddrID)

        iFile.seek(128 + 4)
        data = iFile.read(512 - (128 + 4))
        oFile.write(data)
        oFile.write(secureID)

        iFile.seek(512 + 2)
        data = iFile.read(0x5800 - (512 + 2))
        oFile.write(data)
        oFile.write(partition)

        iFile.seek(0x5800 + 1024)
        data = iFile.read()
        oFile.write(data)
        print("done")

if __name__ == "__main__":
    main()




