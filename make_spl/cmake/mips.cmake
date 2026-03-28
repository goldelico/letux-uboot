if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    set(EXEC_EXTENTION_SUFFIX )
else()
    set(EXEC_EXTENTION_SUFFIX .exe)
endif()

execute_process(COMMAND mips-sde-elf-gcc  -print-libgcc-file-name  OUTPUT_VARIABLE TOOL_CHAINS )
string(REPLACE "/libgcc.a" "" TOOL_CHAINS ${TOOL_CHAINS})
string(REGEX REPLACE "\n$" "" TOOL_CHAINS ${TOOL_CHAINS})


set(LD mips-sde-elf-ld)
set (CMAKE_C_COMPILER "mips-sde-elf-gcc")
set(CPP mips-sde-elf-gcc -E)
set(CC mips-sde-elf-gcc)
set(HOSTCC gcc)
set(LDFLAGS  -G 0 -static -n -nostdlib -EL -m elf32ltsmip)

set(INCLUDE_PATH1 ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(INCLUDE_PATH2 ${CMAKE_CURRENT_SOURCE_DIR}/include2)
set(LIBFDT_ENV_H ${INCLUDE_PATH1}/libfdt_env.h)


set(HOSTCFLAGS -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer)
set(HOSTCFLAGS ${HOSTCFLAGS} -I ${INCLUDE_PATH1} -idirafter ${INCLUDE_PATH1}) #-idirafter ${INCLUDE_PATH2} -idirafter ${INCLUDE_PATH1})
set(HOSTCFLAGS  ${HOSTCFLAGS} -DUSE_HOSTCC -D__KERNEL_STRICT_NAMES)

set(CMAKE_C_FLAGS " -g  -Os   -ffunction-sections -fdata-sections -D__KERNEL__")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -DCONFIG_SYS_TEXT_BASE=${CONFIG_SYS_TEXT_BASE} -DCONFIG_SPL_TEXT_BASE=${CONFIG_SPL_TEXT_BASE} -DCONFIG_SPL_PAD_TO=${CONFIG_SPL_PAD_TO} -DCONFIG_SPL_BUILD -I${INCLUDE_PATH1}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-builtin -ffreestanding -nostdinc -isystem ${TOOL_CHAINS}/include -pipe  -DCONFIG_MIPS")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__MIPS__ -G 0  -EL -msoft-float -std=gnu89 -fno-pic -mno-abicalls -march=mips32r2 -mabi=32 -DCONFIG_32BIT -mno-branch-likely ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wstrict-prototypes -fno-stack-protector -Wno-format-nonliteral -Wno-format-security -fstack-usage")



set(SPL_CPPFLAGS -g -Os -ffunction-sections -fdata-sections -D__KERNEL__)
set(SPL_CPPFLAGS  ${SPL_CPPFLAGS} -ffunction-sections -fdata-sections -DCONFIG_SYS_TEXT_BASE=${CONFIG_SYS_TEXT_BASE} -DCONFIG_SPL_TEXT_BASE=${CONFIG_SPL_TEXT_BASE} -DCONFIG_SPL_PAD_TO=${CONFIG_SPL_PAD_TO} -DCONFIG_SPL_BUILD -I${INCLUDE_PATH1})
set(SPL_CPPFLAGS ${SPL_CPPFLAGS} -fno-builtin -ffreestanding -nostdinc -isystem ${TOOL_CHAINS}/include -pipe    -DCONFIG_MIPS -D__MIPS__ )
set(SPL_CPPFLAGS ${SPL_CPPFLAGS} -G 0 -EL -msoft-float -std=gnu89 -fno-pic -mno-abicalls -march=mips32r2 -mabi=32 -DCONFIG_32BIT -mno-branch-likely)






