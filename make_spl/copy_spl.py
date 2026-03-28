#!/usr/bin/python
# coding:utf-8
import os
import shutil
import sys

uboot_path=sys.argv[1]
output_path=sys.argv[2]

# 保证编译后执行此脚本
sign_file=uboot_path+"/include/config.mk"
if not os.path.exists(sign_file):
    print("Err: Please perform the compilation operation first")
    sys.exit()


# 保证每一次执行都是最新的状态

path_include=output_path+"/include"
path_src=output_path+"/src"
tools_path=output_path+"/tools/"
if os.path.exists(path_include):
    shutil.rmtree(path_include)
    os.mkdir(path_include)

if os.path.exists(path_src):
    shutil.rmtree(path_src)
    os.mkdir(path_src)

if os.path.exists(tools_path):
    shutil.rmtree(tools_path)
    os.mkdir(tools_path)

if not os.path.exists(output_path):
    os.mkdir(output_path)

if not os.path.exists(output_path+"/cmake"):
    os.mkdir(output_path+"/cmake")


# 获取spl下所有 .depend,并进行处理
str_fo=""
from fnmatch import fnmatch
basedir=uboot_path+"/spl"
for root, dirs, files in os.walk(basedir):
     for file in files:
        path = os.path.join(root, file)
        if fnmatch(path, "*.depend"):
            fo=open(path ,"r+")
            str_fo=fo.read()
            str_replace=str_fo.replace("\\","")
            list_tmp=str_replace.split()
            string_o=""
            string_c=""
            for i in list_tmp:
                if ".o:" in i:
                    string_o += str(i) + "\n"
                    continue
                elif "mips-" in i:
                    continue
                elif not "include" in i:
                    string_c += str(i) + "\n"
                    continue
                hfile_path=os.path.dirname(i)
                include_path=hfile_path.split('include')
                if not os.path.exists(i):
                    continue
                src_path=output_path+"/include"+include_path[1]+"/"
                if not os.path.exists(str(src_path)):
                    os.makedirs(str(src_path))
                shutil.copy(i, src_path)

            if len(string_o)==0:
                continue
            list_tmp2=string_o.split()
            cfile_path=os.path.dirname(list_tmp2[0])
            list_cfile=string_c.split()

            for index in list_cfile:
                tmp_cfile_path=cfile_path+"/"+index
                replace_cfile_path=tmp_cfile_path.replace("/spl","",1)
                if not os.path.exists(replace_cfile_path):
                    continue
                split_cfile_path=cfile_path.split('spl',1)
                src_path=output_path+"/src"+split_cfile_path[1]+"/"+index
                tmp_path=os.path.dirname(src_path)
                if not os.path.exists(str(tmp_path)):
                    os.makedirs(str(tmp_path))
                shutil.copy(replace_cfile_path, tmp_path)

            fo.close()


# 拷贝 start.S 和 lds

param_list=[]  # 获取参数列表 固定含义 'ARCH', 'CPU', 'BOARD', 'VENDOR', 'SOC'
config_dfile_path=output_path+"/cmake/config.cmake"
config_sfile_path=uboot_path+"/include/config.mk"
config_file=open(config_dfile_path,'w+')
with open(config_sfile_path,'r') as file:
    for num in range(0,5):
        tmp=file.readline()
        tmp3=tmp.replace("\n","")
        handle_str="set("+tmp3.replace("="," ")+")\n"
        config_file.writelines(handle_str)
        tmp2=tmp.split('=')[1]
        param_list +=tmp2.split()

str_copy1=uboot_path+"/arch/"+param_list[0]+"/cpu/"+param_list[1]+"/"+param_list[4]+"/start.S"
str_copy2=uboot_path+"/arch/"+param_list[0]+"/cpu/"+param_list[1]+"/"+param_list[4]+"/u-boot-spl.lds"
str_copy3=uboot_path+"/include/u-boot/u-boot.lds.h"
shutil.copy(str_copy1,output_path+"/src/arch/"+param_list[0]+"/cpu/"+param_list[1]+"/"+param_list[4]+"/")
shutil.copy(str_copy2,output_path+"/src/arch/"+param_list[0]+"/cpu/"+param_list[1]+"/"+param_list[4]+"/")
shutil.copy(str_copy3,output_path+"/include/u-boot/")




# 处理 autoconf.mk为cmake文件编译做准备
handle_file_path=uboot_path+"/include/autoconf.mk"
write_file_path=output_path+"/cmake/autoconf.cmake"
handle_file = open(handle_file_path,'r')
write_file = open(write_file_path,'w+')

def LineToCmake(line):
    if line.startswith("#"):
        return
    line = line.strip()
    if line.startswith("CONFIG_"):
        d = line.find("=")
        confStr = line[0:d]
        contentStr = line[d+1:]
        isStr = False
        if contentStr.startswith("\""):
            e = contentStr.rfind("\"")
            contentStr = contentStr[1:e]
            isStr = True
        info=""
        isBrackets = False
        for s in contentStr:
            if isBrackets == True and s == "(":
                info = info + "{"
                continue
            if isBrackets == True and s == ")":
                info = info + "}"
                isBrackets = False
                continue
            if(s == "\""):
                info = info + "\\"
            elif (s == "#"):
                 info = info + "\\"
            elif (s =="$"):
                isBrackets = True
            info = info + s
        if isStr:
            info = "\"" + info + "\""
        return (confStr, info)

while True:
    line = handle_file.readline()
    if not line:
        break
    (conf,info) =LineToCmake(line)
    if len(conf):
        str_tmp="set(" + conf + " " + info + ")\n"
        write_file.writelines(str_tmp)

handle_file.close()
write_file.close()



# 拷贝 ingenic_tools 文件
if not os.path.exists(tools_path):
    os.mkdir(tools_path)

libfdt_env_h=uboot_path+"/include/libfdt_env.h"
shutil.copy(libfdt_env_h,path_include)

ingenic_tools_path=uboot_path+"/tools/ingenic-tools/"
ingenic_tools_fo=open(ingenic_tools_path+".spl_source_tools_depend",'r')
ingenic_tools_str=ingenic_tools_fo.read()
ingenic_tools_cmake=output_path+"/cmake/ingenic_tools.cmake"
ingenic_tools_fo2=open(ingenic_tools_cmake,'w+')

for i in ingenic_tools_str.split():
    shutil.copy(ingenic_tools_path+i,tools_path)
    if "spl_params_fixer" in i:
        str_tmp="set(SPL_PARRMS_FIXER  "+i+")\n"
        ingenic_tools_fo2.writelines(str_tmp)
        continue
    if "sfc_nor_builtin_params" in i:
        str_tmp="set(SFC_NOR_BUILTIN_PARAMS  "+i+")\n"
        ingenic_tools_fo2.writelines(str_tmp)
        continue
    if "nor_device" in i:
        str_tmp="set(SFC_NOR_DEVICE  "+i+")\n"
        ingenic_tools_fo2.writelines(str_tmp)
        continue
    if "sfc_nand_builtin_params" in i:
        str_tmp="set(SFC_NAND_BUILTIN_PARAMS  "+i+")\n"
        ingenic_tools_fo2.writelines(str_tmp)
        continue
    if "nand_device" in i:
        str_tmp="set(SFC_NAND_NAND_DEVICE  "+i+")\n"
        ingenic_tools_fo2.writelines(str_tmp)
        continue

    str_tmp=i.replace(".c","")
    str_tmp2="set("+str_tmp.upper()+"  "+i+")\n"
    ingenic_tools_fo2.writelines(str_tmp2)

ingenic_tools_fo.close()
ingenic_tools_fo2.close()

# 拷贝cmake 所需文件

for cmake_file in os.listdir(uboot_path+"/make_spl"):
    full_file_name = os.path.join(uboot_path+"/make_spl", cmake_file)
    if "copy_spl" in cmake_file:
        continue
    if os.path.isfile(full_file_name):
        shutil.copy(full_file_name, output_path)

for cmake_file in os.listdir(uboot_path+"/make_spl/cmake"):
    full_file_name = os.path.join(uboot_path+"/make_spl/cmake",cmake_file)
    if os.path.isfile(full_file_name):
        if not os.path.exists(output_path+"/cmake"):
            os.mkdir(output_path+"/cmake")
        shutil.copy(full_file_name, output_path+"/cmake")


for cmake_file in os.listdir(uboot_path+"/make_spl/.vscode"):
    full_file_name = os.path.join(uboot_path+"/make_spl/.vscode",cmake_file)
    if os.path.isfile(full_file_name):
        if not os.path.exists(output_path+"/.vscode"):
            os.mkdir(output_path+"/.vscode")
        shutil.copy(full_file_name, output_path+"/.vscode")



print ("copy spl files ok !")




