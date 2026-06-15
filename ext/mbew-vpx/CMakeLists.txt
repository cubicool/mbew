project(mbew-vpx)

set(MBEW_VPX_SRC_FILES
    "libvpx/vpx/src/vpx_decoder.c"
    "libvpx/vpx/src/vpx_encoder.c"
    "libvpx/vpx/src/vpx_codec.c"
    "libvpx/vpx/src/vpx_image.c"
    "libvpx/vpx_mem/vpx_mem.c"
    "libvpx/vpx_scale/generic/vpx_scale.c"
    "libvpx/vpx_scale/generic/yv12config.c"
    "libvpx/vpx_scale/generic/yv12extend.c"
    "libvpx/vpx_scale/generic/gen_scalers.c"
    "libvpx/vpx_scale/vpx_scale_rtcd.c"
    "libvpx/vpx_dsp/prob.c"
    "libvpx/vpx_dsp/bitreader.c"
    "libvpx/vpx_dsp/bitreader_buffer.c"
    "libvpx/vpx_dsp/intrapred.c"
    "libvpx/vpx_dsp/add_noise.c"
    "libvpx/vpx_dsp/deblock.c"
    "libvpx/vpx_dsp/vpx_convolve.c"
    "libvpx/vpx_dsp/x86/vpx_asm_stubs.c"
    "libvpx/vpx_dsp/x86/vpx_subpixel_8t_intrin_avx2.c"
    "libvpx/vpx_dsp/x86/vpx_subpixel_8t_intrin_ssse3.c"
    "libvpx/vpx_dsp/loopfilter.c"
    "libvpx/vpx_dsp/x86/loopfilter_sse2.c"
    "libvpx/vpx_dsp/x86/loopfilter_avx2.c"
    "libvpx/vpx_dsp/inv_txfm.c"
    "libvpx/vpx_dsp/x86/inv_txfm_sse2.c"
    "libvpx/vpx_dsp/variance.c"
    "libvpx/vpx_dsp/x86/variance_sse2.c"
    "libvpx/vpx_dsp/x86/variance_avx2.c"
    "libvpx/vpx_dsp/x86/variance_impl_avx2.c"
    "libvpx/vpx_dsp/vpx_dsp_rtcd.c"
    "libvpx/vpx_util/vpx_thread.c"
    "libvpx/vp8/common/alloccommon.c"
    "libvpx/vp8/common/blockd.c"
    "libvpx/vp8/common/copy_c.c"
    "libvpx/vp8/common/dequantize.c"
    "libvpx/vp8/common/entropy.c"
    "libvpx/vp8/common/entropymode.c"
    "libvpx/vp8/common/entropymv.c"
    "libvpx/vp8/common/extend.c"
    "libvpx/vp8/common/filter.c"
    "libvpx/vp8/common/findnearmv.c"
    "libvpx/vp8/common/generic/systemdependent.c"
    "libvpx/vp8/common/idct_blk.c"
    "libvpx/vp8/common/idctllm.c"
    "libvpx/vp8/common/rtcd.c"
    "libvpx/vp8/common/vp8_loopfilter.c"
    "libvpx/vp8/common/loopfilter_filters.c"
    "libvpx/vp8/common/mbpitch.c"
    "libvpx/vp8/common/modecont.c"
    "libvpx/vp8/common/quant_common.c"
    "libvpx/vp8/common/reconinter.c"
    "libvpx/vp8/common/reconintra.c"
    "libvpx/vp8/common/reconintra4x4.c"
    "libvpx/vp8/common/setupintrarecon.c"
    "libvpx/vp8/common/swapyv12buffer.c"
    "libvpx/vp8/common/treecoder.c"
    "libvpx/vp8/common/x86/filter_x86.c"
    "libvpx/vp8/common/x86/vp8_asm_stubs.c"
    "libvpx/vp8/common/x86/loopfilter_x86.c"
    "libvpx/vp8/common/mfqe.c"
    "libvpx/vp8/common/postproc.c"
    "libvpx/vp8/common/x86/idct_blk_mmx.c"
    "libvpx/vp8/common/x86/idct_blk_sse2.c"
    "libvpx/vp8/vp8_dx_iface.c"
    "libvpx/vp8/decoder/dboolhuff.c"
    "libvpx/vp8/decoder/decodemv.c"
    "libvpx/vp8/decoder/decodeframe.c"
    "libvpx/vp8/decoder/detokenize.c"
    "libvpx/vp8/decoder/onyxd_if.c"
    "libvpx/vp8/decoder/threading.c"
    "libvpx/vp9/common/vp9_alloccommon.c"
    "libvpx/vp9/common/vp9_blockd.c"
    "libvpx/vp9/common/vp9_entropy.c"
    "libvpx/vp9/common/vp9_entropymode.c"
    "libvpx/vp9/common/vp9_entropymv.c"
    "libvpx/vp9/common/vp9_frame_buffers.c"
    "libvpx/vp9/common/vp9_idct.c"
    "libvpx/vp9/common/vp9_filter.c"
    "libvpx/vp9/common/vp9_pred_common.c"
    "libvpx/vp9/common/vp9_rtcd.c"
    "libvpx/vp9/common/vp9_scale.c"
    "libvpx/vp9/common/vp9_seg_common.c"
    "libvpx/vp9/common/vp9_tile_common.c"
    "libvpx/vp9/common/vp9_loopfilter.c"
    "libvpx/vp9/common/vp9_common_data.c"
    "libvpx/vp9/common/vp9_mvref_common.c"
    "libvpx/vp9/common/vp9_quant_common.c"
    "libvpx/vp9/common/vp9_reconinter.c"
    "libvpx/vp9/common/vp9_reconintra.c"
    "libvpx/vp9/common/vp9_scan.c"
    "libvpx/vp9/common/vp9_thread_common.c"
    "libvpx/vp9/common/x86/vp9_idct_intrin_sse2.c"
    "libvpx/vp9/decoder/vp9_decodeframe.c"
    "libvpx/vp9/decoder/vp9_decodemv.c"
    "libvpx/vp9/decoder/vp9_decoder.c"
    "libvpx/vp9/decoder/vp9_detokenize.c"
    "libvpx/vp9/decoder/vp9_dsubexp.c"
    "libvpx/vp9/decoder/vp9_dthread.c"
    "libvpx/vp9/vp9_dx_iface.c"
)

set(MBEW_VPX_ASM_FILES
    "libvpx/vpx_ports/emms.asm"
    "libvpx/vpx_dsp/x86/intrapred_sse2.asm"
    "libvpx/vpx_dsp/x86/intrapred_ssse3.asm"
    "libvpx/vpx_dsp/x86/vpx_subpixel_8t_ssse3.asm"
    "libvpx/vpx_dsp/x86/add_noise_sse2.asm"
    "libvpx/vpx_dsp/x86/deblock_sse2.asm"
    "libvpx/vpx_dsp/x86/vpx_subpixel_8t_sse2.asm"
    "libvpx/vpx_dsp/x86/vpx_subpixel_bilinear_sse2.asm"
    "libvpx/vpx_dsp/x86/vpx_subpixel_bilinear_ssse3.asm"
    "libvpx/vpx_dsp/x86/vpx_convolve_copy_sse2.asm"
    "libvpx/vpx_dsp/x86/inv_wht_sse2.asm"
    "libvpx/vpx_dsp/x86/subpel_variance_sse2.asm"
    "libvpx/vp8/common/x86/dequantize_mmx.asm"
    "libvpx/vp8/common/x86/idctllm_mmx.asm"
    "libvpx/vp8/common/x86/recon_mmx.asm"
    "libvpx/vp8/common/x86/subpixel_mmx.asm"
    "libvpx/vp8/common/x86/copy_sse2.asm"
    "libvpx/vp8/common/x86/idctllm_sse2.asm"
    "libvpx/vp8/common/x86/recon_sse2.asm"
    "libvpx/vp8/common/x86/subpixel_sse2.asm"
    "libvpx/vp8/common/x86/loopfilter_sse2.asm"
    "libvpx/vp8/common/x86/iwalsh_sse2.asm"
    "libvpx/vp8/common/x86/copy_sse3.asm"
    "libvpx/vp8/common/x86/subpixel_ssse3.asm"
    "libvpx/vp8/common/x86/mfqe_sse2.asm"
)

set(MBEW_ARCH "x86_64")
set(MBEW_VPX_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/libvpx")

if(MBEW32)
    set(MBEW_ARCH "x86")
else()
    list(APPEND MBEW_VPX_ASM_FILES
        "libvpx/vpx_ports/x86_abi_support.asm"
        "libvpx/vpx_dsp/x86/inv_txfm_ssse3_x86_64.asm"
        "libvpx/vpx_dsp/x86/ssim_opt_x86_64.asm"
        "libvpx/vp8/common/x86/loopfilter_block_sse2_x86_64.asm"
    )
endif()

if(NOT WIN32)
    list(APPEND MBEW_VPX_SRC_FILES "os-linux/${MBEW_ARCH}/vpx_config.c")
    list(APPEND MBEW_VPX_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/os-linux/${MBEW_ARCH}")
else()
    list(APPEND MBEW_VPX_SRC_FILES "os-windows/${MBEW_ARCH}/vpx_config.c")
    list(APPEND MBEW_VPX_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/os-windows/${MBEW_ARCH}")
endif()

if(NOT WIN32)
    set(CMAKE_ASM_NASM_COMPILER yasm)
    set(CMAKE_ASM_NASM_FLAGS "-DPIC")
    enable_language(ASM_NASM)

    add_library(mbew-vpx STATIC
        ${MBEW_VPX_SRC_FILES}
        ${MBEW_VPX_ASM_FILES}
    )

    target_compile_options(mbew-vpx PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-mmmx>
        $<$<COMPILE_LANGUAGE:C>:-mavx>
        $<$<COMPILE_LANGUAGE:C>:-mavx2>
        $<$<COMPILE_LANGUAGE:C>:-msse>
        $<$<COMPILE_LANGUAGE:C>:-msse2>
        $<$<COMPILE_LANGUAGE:C>:-msse3>
        $<$<COMPILE_LANGUAGE:C>:-mssse3>
        $<$<COMPILE_LANGUAGE:C>:-msse4.1>
    )
    target_link_libraries(mbew-vpx PUBLIC m)

# There is a known issue with assembly files and Visual Studio that CMake has not yet resolved
# (assembly support only works for Makefile-based setups). A minimal custom command is used instead.
else()
    set(YASM_PATH "" CACHE FILEPATH "Location of the YASM executable/binary.")
    set(MBEW_ASM_ARCH "win64")

    if(MBEW32)
        set(MBEW_ASM_ARCH "win32")
    endif()

    foreach(ID ${MBEW_VPX_INCLUDE_DIRS})
        list(APPEND MBEW_VPX_ASM_INC "-I${ID}")
    endforeach()

    foreach(AF ${MBEW_VPX_ASM_FILES})
        get_filename_component(AF_NAME ${AF} NAME_WE)
        get_filename_component(AF_ABS ${AF} ABSOLUTE)
        get_filename_component(AF_DIR ${AF} DIRECTORY)

        set(AF_BINDIR "CMakeFiles/mbew-vpx.dir/${AF_DIR}")
        set(AF_OBJ "${AF_BINDIR}/${AF_NAME}.asm.o")

        list(APPEND MBEW_VPX_ASM_OBJS "${AF_OBJ}")

        add_custom_command(
            OUTPUT "${AF_OBJ}"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${AF_BINDIR}
            COMMAND ${YASM_PATH} ${AF_ABS} -f ${MBEW_ASM_ARCH} -o ${AF_OBJ} ${MBEW_VPX_ASM_INC}
            DEPENDS "${AF_ABS}"
            COMMENT "Custom YASM compilation ${AF_OBJ}"
        )
    endforeach()

    add_library(mbew-vpx STATIC
        ${MBEW_VPX_SRC_FILES}
        ${MBEW_VPX_ASM_OBJS}
    )

    target_compile_options(mbew-vpx PRIVATE /arch:AVX)
endif()

target_include_directories(mbew-vpx PUBLIC ${MBEW_VPX_INCLUDE_DIRS})
