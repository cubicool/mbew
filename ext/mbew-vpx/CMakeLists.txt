PROJECT(mbew-vpx)

SET(MBEW_VPX_SRC_FILES
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

SET(MBEW_VPX_ASM_FILES
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

SET(MBEW_VPX_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/libvpx")
SET(MBEW_ARCH "x86_64")

IF(MBEW32)
	SET(MBEW_ARCH "x86")

ELSE()
	LIST(APPEND MBEW_VPX_ASM_FILES
		"libvpx/vpx_ports/x86_abi_support.asm"
		"libvpx/vpx_dsp/x86/inv_txfm_ssse3_x86_64.asm"
		"libvpx/vpx_dsp/x86/ssim_opt_x86_64.asm"
		"libvpx/vp8/common/x86/loopfilter_block_sse2_x86_64.asm"
	)
ENDIF()

IF(NOT WIN32)
	SET(CMAKE_C_FLAGS "-mmmx -mavx -mavx2 -msse -msse2 -msse3 -mssse3 -msse4.1")

	LIST(APPEND MBEW_VPX_SRC_FILES "os-linux/${MBEW_ARCH}/vpx_config.c")
	LIST(APPEND MBEW_VPX_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/os-linux/${MBEW_ARCH}")
ELSE()
	SET(CMAKE_C_FLAGS "/arch:AVX")
	SET(YASM_PATH "" CACHE FILEPATH "Location of the YASM executable/binary.")

	LIST(APPEND MBEW_VPX_SRC_FILES "os-windows/${MBEW_ARCH}/vpx_config.c")
	LIST(APPEND MBEW_VPX_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/os-windows/${MBEW_ARCH}")
ENDIF()

SET(MBEW_VPX_PUBLIC_HEADER_FILES)
SET(MBEW_VPX_PRIVATE_HEADER_FILES)

SET(MBEW_VPX_HEADER_FILES
	${MBEW_VPX_PUBLIC_HEADER_FILES}
	${MBEW_VPX_PRIVATE_HEADER_FILES}
)

INCLUDE_DIRECTORIES(${MBEW_VPX_INCLUDE_DIRS})

SOURCE_GROUP("Header Files" FILES ${MBEW_VPX_HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${MBEW_VPX_SRC_FILES})
SOURCE_GROUP("ASM Files" FILES ${MBEW_VPX_ASM_FILES})

IF(NOT WIN32)
	ENABLE_LANGUAGE(ASM_NASM)

	ADD_LIBRARY(mbew-vpx STATIC
		${MBEW_VPX_HEADER_FILES}
		${MBEW_VPX_SRC_FILES}
		${MBEW_VPX_ASM_FILES}
	)

# There is a known issue with assembly files and Visual Studio that CMake has not yet resolved (that
# is, assembly support only works for "Makefile-based" setups). I have created a minimal custom
# command for doing this below.
ELSE()
	SET(MBEW_ASM_ARCH "win64")

	IF(MBEW32)
		SET(MBEW_ASM_ARCH "win32")
	ENDIF()

	# Covert our include directories into the -IFOO -IBAR format YASM expects.
	FOREACH(ID ${MBEW_VPX_INCLUDE_DIRS})
		LIST(APPEND MBEW_VPX_ASM_INC "-I${ID}")
	ENDFOREACH()

	# Iterate over each .asm file and create a custom build command for it.
	FOREACH(AF ${MBEW_VPX_ASM_FILES})
		GET_FILENAME_COMPONENT(AF_NAME ${AF} NAME_WE)
		GET_FILENAME_COMPONENT(AF_ABS ${AF} ABSOLUTE)
		GET_FILENAME_COMPONENT(AF_DIR ${AF} DIRECTORY)

		SET(AF_BINDIR "CMakeFiles/mbew-vpx.dir/${AF_DIR}")
		SET(AF_OBJ "${AF_BINDIR}/${AF_NAME}.asm.o")

		# MESSAGE("==============================================")
		# MESSAGE("  >> AF        = ${AF}")
		# MESSAGE("  >> AF_NAME   = ${AF_NAME}")
		# MESSAGE("  >> AF_ABS    = ${AF_ABS}")
		# MESSAGE("  >> AF_DIR    = ${AF_DIR}")
		# MESSAGE("  >> AF_BINDIR = ${AF_BINDIR}")
		# MESSAGE("  >> AF_OBJ    = ${AF_OBJ}")

		LIST(APPEND MBEW_VPX_ASM_OBJS "${AF_OBJ}")

		ADD_CUSTOM_COMMAND(
			OUTPUT "${AF_OBJ}"
			PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${AF_BINDIR}
			COMMAND ${YASM_PATH} ${AF_ABS} -f ${MBEW_ASM_ARCH} -o ${AF_OBJ} ${MBEW_VPX_ASM_INC}
			DEPENDS "${AF_ABS}"
			COMMENT "Custom YASM compilation ${AF_OBJ}"
		)
	ENDFOREACH()

	ADD_LIBRARY(mbew-vpx STATIC
		${MBEW_VPX_HEADER_FILES}
		${MBEW_VPX_SRC_FILES}
		${MBEW_VPX_ASM_OBJS}
	)
ENDIF()
