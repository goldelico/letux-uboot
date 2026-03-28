cmake_minimum_required(VERSION 3.20.1)

PROJECT (make_spl)

include(cmake/autoconf.cmake)
include(cmake/mips.cmake)
include(cmake/config.cmake)
include(cmake/ingenic_tools.cmake)
message("using toolchain " ${CMAKE_TOOLCHAIN_FILE})

set(DEP_VAR )
if( ${CONFIG_SPL_PARAMS_FIXER} MATCHES "y")
    set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tools/${SPL_PARRMS_FIXER})
    set(OUTPUT spl_params_fixer)
    add_custom_target( ${OUTPUT}_e ALL DEPENDS HOST_TOOLS_BUILD.${OUTPUT})
    include(cmake/host-tools.cmake)
    set(DEP_VAR ${DEP_VAR} ${OUTPUT}_e)
    set(SPL_PARRMS_STR ${CMAKE_CURRENT_BINARY_DIR}/spl_params_fixer${EXEC_EXTENTION_SUFFIX} u-boot-spl.bin u-boot-spl.bin 256 256)
endif()

if( ${CONFIG_SPI_SPL_CHECK} MATCHES "y")
    set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tools/${SPI_CHECKSUM})
    set(OUTPUT spi_checksum)
    add_custom_target( ${OUTPUT}_e ALL DEPENDS HOST_TOOLS_BUILD.${OUTPUT})
    include(cmake/host-tools.cmake)
    set(DEP_VAR ${DEP_VAR} ${OUTPUT}_e)
    set(SPI_CHECKSUM_STR ${CMAKE_CURRENT_BINARY_DIR}/spi_checksum${EXEC_EXTENTION_SUFFIX} u-boot-spl.bin)
endif()


if( ${CONFIG_MMC_SPL_PARAMS} MATCHES "y")
    set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tools/${MMC_PARAMS})
    set(OUTPUT mmc_params)
    add_custom_target( ${OUTPUT}_e ALL DEPENDS HOST_TOOLS_BUILD.${OUTPUT})
    include(cmake/host-tools.cmake)
    set(DEP_VAR ${DEP_VAR} ${OUTPUT}_e)
endif()


if( ${CONFIG_SPL_PAD_TO_BLOCK} MATCHES "y")
    set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tools/${SPL_PAD_TO_BLOCK})
    set(OUTPUT spl_pad_to_block)
    add_custom_target( ${OUTPUT}_e ALL DEPENDS HOST_TOOLS_BUILD.${OUTPUT})
    include(cmake/host-tools.cmake)
    set(DEP_VAR ${DEP_VAR} ${OUTPUT}_e)
    set(SPL_PDA_STR ${CMAKE_CURRENT_BINARY_DIR}/spl_pad_to_block${EXEC_EXTENTION_SUFFIX} u-boot-spl.bin)
endif()


if( ${CONFIG_NOR_BUILTIN_PARAMS} MATCHES "y")
    set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tools/${SFC_NOR_BUILTIN_PARAMS}   ${CMAKE_CURRENT_SOURCE_DIR}/tools/${SFC_NOR_DEVICE})
    set(OUTPUT sfc_nor_builtin_params)
    add_custom_target( ${OUTPUT}_e ALL DEPENDS HOST_TOOLS_BUILD.${OUTPUT})
    include(cmake/host-tools.cmake)
    set(DEP_VAR ${DEP_VAR} ${OUTPUT}_e)
endif()


if( ${CONFIG_NAND_BUILTIN_PARAMS} MATCHES "y")
    set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tools/${SFC_NAND_BUILTIN_PARAMS}   ${CMAKE_CURRENT_SOURCE_DIR}/tools/${SFC_NAND_NAND_DEVICE})
    set(OUTPUT sfc_nand_builtin_params)
    add_custom_target( ${OUTPUT}_e ALL DEPENDS HOST_TOOLS_BUILD.${OUTPUT})
    include(cmake/host-tools.cmake)
    set(DEP_VAR ${DEP_VAR} ${OUTPUT}_e)
endif()



file(GLOB SPL_SRC_FILES_SOC ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/mips/cpu/${CPU}/${SOC}/*.c)
file(GLOB SPL_SRC_FILES_common_spl ${CMAKE_CURRENT_SOURCE_DIR}/src/common/spl/*.c)
file(GLOB SPL_SRC_FILES_gpio  ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/gpio/*.c)
file(GLOB SPL_SRC_FILES_lib ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/*.c)
file(GLOB SPL_SRC_FILES_BOARD ${CMAKE_CURRENT_SOURCE_DIR}/src/board/${VENDOR}/${BOARD}/*.c)
file(GLOB SPL_SRC_FILES_serial ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/serial/*.c)
file(GLOB SPL_SRC_FILES_CPU ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/mips/cpu/${CPU}/*.c)


add_library(spl STATIC
    ${SPL_SRC_FILES_CPU}
    ${SPL_SRC_FILES_SOC}
    ${SPL_SRC_FILES_BOARD}
    ${SPL_SRC_FILES_common_spl}
    ${SPL_SRC_FILES_gpio}
    ${SPL_SRC_FILES_serial}
    ${SPL_SRC_FILES_lib}
    )

target_include_directories(spl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


add_custom_target(start_S ALL DEPENDS arch/mips/cpu/${CPU}/${SOC}/start.o)
add_custom_command(OUTPUT arch/mips/cpu/${CPU}/${SOC}/start.o
    COMMAND ${CMAKE_COMMAND} -E make_directory arch/mips/cpu/${CPU}/${SOC}
    COMMAND ${CC} -D__ASSEMBLY__ ${SPL_CPPFLAGS} -I ${CMAKE_CURRENT_SOURCE_DIR} -o arch/mips/cpu/${CPU}/${SOC}/start.o ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/mips/cpu/${CPU}/${SOC}/start.S -c
    BYPRODUCTS arch/mips/cpu/${CPU}/${SOC}/start.o arch/mips/cpu/${CPU}/${SOC}/ arch/mips/cpu/${CPU}/ arch/mips/cpu/ arch/mips/ arch/
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/mips/cpu/${CPU}/${SOC}/start.S
        COMMENT "Generating start.o"
        VERBATIM
    )



set(LDPPFLAGS -include ${CMAKE_CURRENT_SOURCE_DIR}/include/u-boot/u-boot.lds.h -include ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h -DCPUDIR=arch/mips/cpu/${CPU})

add_custom_target(u-boot-spl_lds ALL DEPENDS u-boot-spl.lds)
add_custom_command(OUTPUT u-boot-spl.lds
    COMMAND ${CPP} ${SPL_CPPFLAGS} -include ${CMAKE_CURRENT_SOURCE_DIR}/include/u-boot/u-boot.lds.h -include ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h -DCPUDIR=arch/mips/cpu/${CPU} -I${spl_path}. -ansi -D__ASSEMBLY__ -P - < ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/mips/cpu/${CPU}/${SOC}/u-boot-spl.lds > u-boot-spl.lds
        BYPRODUCTS u-boot-spl.lds
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/mips/cpu/${CPU}/${SOC}/u-boot-spl.lds
        COMMENT "Generating ${spl_path}/u-boot-spl.lds"
        VERBATIM
    )



set(spl_ldflgs -T u-boot-spl.lds --gc-sections --gc-sections -Bstatic -Ttext ${CONFIG_SPL_TEXT_BASE})
set(__start arch/mips/cpu/${CPU}/${SOC}/start.o)
set(__LIBS libspl.a)
set(PLATFORM_LIBS -L ${TOOL_CHAINS}/sof -lgcc)

add_custom_target(uboot-spl ALL DEPENDS u-boot-spl)
add_custom_command(OUTPUT u-boot-spl
        COMMAND ${LD} ${LDFLAGS}  ${spl_ldflgs} ${__start} --start-group ${__LIBS} --end-group ${PLATFORM_LIBS} -Map    u-boot-spl.map -o u-boot-spl
        BYPRODUCTS u-boot-spl u-boot-spl.map
        DEPENDS spl u-boot-spl_lds
        COMMENT "Generating ${spl_path}/u-boot-spl"
        VERBATIM
    )


set(OBJCFLAGS --remove-section=.dynsym --gap-fill=0xff)
set(OBJCPY mips-sde-elf-objcopy)
add_custom_target(uboot-spl_bin ALL DEPENDS u-boot-spl.bin)





add_custom_command(OUTPUT u-boot-spl.bin
    COMMAND ${OBJCPY} ${OBJCFLAGS} -O binary u-boot-spl  u-boot-spl.bin
    COMMAND ${SPL_PDA_STR}
    COMMAND ${SPL_PARRMS_STR}
    COMMAND ${SPI_CHECKSUM_STR}
    BYPRODUCTS u-boot-spl.bin
    COMMENT "Generating u-boot-spl.bin"
    DEPENDS u-boot-spl ${DEP_VAR}
    VERBATIM
    )


add_custom_target(readd_uboot-spl_bin ALL DEPENDS u-boot-spl.bin)
if(${CONFIG_MMC_SPL_PARAMS} MATCHES "y")
    add_custom_command(OUTPUT u-boot-spl.bin
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mmc_params u-boot-spl.bin
            BYPRODUCTS u-boot-spl.bin
            DEPENDS uboot-spl_bin mmc_params_e
            COMMENT "Generating add mmc_params option"
            VERBATIM
        )
    set(ADD_OPTION y)
endif()



if(${CONFIG_NOR_BUILTIN_PARAMS} MATCHES "y")
    add_custom_command(OUTPUT u-boot-spl.bin
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sfc_nor_builtin_params u-boot-spl.bin u-boot-spl.bin ${CONFIG_SPIFLASH_PART_OFFSET}
            BYPRODUCTS u-boot-spl.bin
            DEPENDS uboot-spl_bin sfc_nor_builtin_params_e
            COMMENT "Generating add sfc_nor_builtin_params option"
            VERBATIM
        )
    set(ADD_OPTION y)
endif()


if(${CONFIG_NAND_BUILTIN_PARAMS} MATCHES "y")
    add_custom_command(OUTPUT u-boot-spl.bin
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sfc_nand_builtin_params u-boot-spl.bin u-boot-spl.bin ${CONFIG_SPIFLASH_PART_OFFSET}
            BYPRODUCTS u-boot-spl.bin
            DEPENDS uboot-spl_bin sfc_nand_builtin_params_e
            COMMENT "Generating add sfc_nand_builtin_params option"
            VERBATIM
        )
    set(ADD_OPTION y)
endif()

if(${ADD_OPTION} MATCHES "y")
    set(DEP_SPL_PAD readd_uboot-spl_bin)
else()
    set(DEP_SPL_PAD uboot-spl_bin)
endif()


add_custom_target(u-boot-spl-pad.bin ALL
    COMMAND ${OBJCPY} ${OBJCFLAGS} --pad-to=${CONFIG_SPL_PAD_TO} -I binary -O binary u-boot-spl.bin u-boot-spl-pad.bin
        BYPRODUCTS u-boot-spl-pad.bin
        DEPENDS ${DEP_SPL_PAD}
        COMMENT " Generating u-boot-spl-pad.bin"
    )



