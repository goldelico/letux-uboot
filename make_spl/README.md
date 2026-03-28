# SPL 代码生成，用于生成spl所需文件

通过在Linux环境下编译，然后分析依赖文件，提取文件到out_spl_source目录下由于路径没有重新定位，需要内置编译才能提取

**注意: 提取文件的时候,未提取工具所需文件，当改动DDR或CPU频率时需要重新提取**

## 目录结构

```sh
.
|-- CMakeLists.txt  --cmake 文件
|-- cmake           --cmake 所需文件目录
|-- copy_spl.py     --拷贝spl所需文件的python文件
|-- include         --拷贝后include文件目录
|-- mips-gcc-sde-elf.cmake
|-- readme
|-- src             --拷贝后的源文件目录
`-- tools           --拷贝后的工具目录
```

## 若有 linux 环境可执行

打开uboot目录执行 make 板级文件 例如 make panda_rtos_sfc_nor

然后再执行 make spl-source 会将spl所需要的文件拷贝到 out_spl_source 目录下

### 直接拿到拷贝好的spl目录文件

1. 命令行编译
linux:

```sh
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../mips-gcc-sde-elf.cmake ..
make
```

windows:

```sh
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../mips-gcc-sde-elf.cmake -G "MinGW Makefiles" ../
mingw32-make
```

2. vscode 编译

根据 系统环境选择 linux or windows 对应的 cmake-kits
点击底部中间的build

**最终会在 out_spl_source/build 目录下生成 u-boot-spl-pad.bin 进行烧录即可**

