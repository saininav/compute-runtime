#
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

project(ocloc_tests)


set(IGDRCL_SRCS_cloc
${NEO_SOURCE_DIR}/offline_compiler/decoder/binary_decoder.cpp
${NEO_SOURCE_DIR}/offline_compiler/decoder/binary_encoder.cpp
${NEO_SOURCE_DIR}/offline_compiler/offline_compiler.cpp
)

set(IGDRCL_SRCS_offline_compiler_mock
${CMAKE_CURRENT_SOURCE_DIR}/decoder/mock/mock_decoder.h
${CMAKE_CURRENT_SOURCE_DIR}/decoder/mock/mock_encoder.h
${CMAKE_CURRENT_SOURCE_DIR}/decoder/mock/mock_iga_wrapper.h
${CMAKE_CURRENT_SOURCE_DIR}/mock/mock_offline_compiler.h
${CMAKE_CURRENT_SOURCE_DIR}/mock/mock_sip_ocloc_tests.cpp
)

set(CLOC_SRCS_UTILITIES
  ${NEO_SOURCE_DIR}/offline_compiler/utilities/safety_caller.h
  ${NEO_SOURCE_DIR}/offline_compiler/utilities//get_current_dir.h
)

if(WIN32)
  list(APPEND CLOC_SRCS_UTILITIES
    ${NEO_SOURCE_DIR}/offline_compiler/utilities/windows/safety_caller_windows.cpp
    ${NEO_SOURCE_DIR}/offline_compiler/utilities/windows/safety_guard_windows.h
    ${NEO_SOURCE_DIR}/offline_compiler/utilities/windows/seh_exception.cpp
    ${NEO_SOURCE_DIR}/offline_compiler/utilities/windows/seh_exception.h
    ${NEO_SOURCE_DIR}/offline_compiler/utilities/windows/get_current_dir_windows.cpp
  )
else()
  list(APPEND CLOC_SRCS_UTILITIES
    ${NEO_SOURCE_DIR}/offline_compiler/utilities/linux/safety_caller_linux.cpp
    ${NEO_SOURCE_DIR}/offline_compiler/utilities/linux/safety_guard_linux.h
    ${NEO_SOURCE_DIR}/offline_compiler/utilities/linux/get_current_dir_linux.cpp
  )
endif()

get_property(IGDRCL_SRCS_tests_compiler_mocks GLOBAL PROPERTY IGDRCL_SRCS_tests_compiler_mocks)

set(IGDRCL_SRCS_offline_compiler_tests
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/decoder/decoder_tests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/decoder/encoder_tests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/environment.h
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/offline_compiler_tests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/offline_compiler_tests.h
  ${NEO_SOURCE_DIR}/core/helpers/abort.cpp
  ${NEO_SOURCE_DIR}/core/helpers/file_io.cpp
  ${NEO_SOURCE_DIR}/core/memory_manager/deferred_deleter.cpp
  ${NEO_SOURCE_DIR}/core/memory_manager/deferred_deleter.h
  ${NEO_SOURCE_DIR}/unit_tests/helpers/test_files.cpp
  ${IGDRCL_SRCS_cloc}
  ${IGDRCL_SRCS_offline_compiler_mock}
  ${IGDRCL_SRCS_tests_compiler_mocks}
  ${CLOC_SRCS_LIB}
  ${CLOC_SRCS_UTILITIES}
)

if(WIN32)
  list(APPEND IGDRCL_SRCS_offline_compiler_tests
    ${NEO_SOURCE_DIR}/core/os_interface/windows/os_thread_win.cpp
  )
else()
  list(APPEND IGDRCL_SRCS_offline_compiler_tests
    ${NEO_SOURCE_DIR}/core/os_interface/linux/os_thread_linux.cpp
  )
endif()

link_directories(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_executable(ocloc_tests ${IGDRCL_SRCS_offline_compiler_tests})

target_include_directories(ocloc_tests PRIVATE
  $<TARGET_PROPERTY:ocloc,INCLUDE_DIRECTORIES>
)
target_compile_definitions(ocloc_tests PUBLIC MOCKABLE_VIRTUAL=virtual $<TARGET_PROPERTY:ocloc,INTERFACE_COMPILE_DEFINITIONS>)

target_link_libraries(ocloc_tests gmock-gtest elflib)

if(WIN32)
    target_link_libraries(ocloc_tests dbghelp)
endif()

if(UNIX)
  target_link_libraries(ocloc_tests dl pthread)
endif()

get_property(CLOC_FOLDER TARGET ocloc PROPERTY FOLDER)
get_property(CLOC_COMPILE_FLAGS TARGET ocloc PROPERTY COMPILE_FLAGS)
set_property(TARGET ocloc_tests PROPERTY FOLDER ${CLOC_FOLDER})
set_property(TARGET ocloc_tests APPEND_STRING PROPERTY COMPILE_FLAGS ${CLOC_COMPILE_FLAGS})

string(TOLOWER ${DEFAULT_TESTED_PLATFORM} CLOC_DEFAULT_DEVICE)

add_custom_target(run_ocloc_tests ALL
  DEPENDS ocloc_tests
)
macro(macro_for_each_platform)
  if("${PLATFORM_IT_LOWER}" STREQUAL "${CLOC_DEFAULT_DEVICE}")
    foreach(PLATFORM_TYPE ${PLATFORM_TYPES})
      if(${PLATFORM_IT}_IS_${PLATFORM_TYPE})
        get_family_name_with_type(${GEN_TYPE} ${PLATFORM_TYPE})
        add_dependencies(run_ocloc_tests prepare_test_kernels_${family_name_with_type})
        neo_copy_test_files(copy_test_files_${family_name_with_type} ${family_name_with_type})
        add_dependencies(run_ocloc_tests copy_test_files_${family_name_with_type})
        set(run_tests_cmd ocloc_tests --device ${CLOC_DEFAULT_DEVICE} --family_type ${family_name_with_type})
      endif()
    endforeach()
  endif()
endmacro()
macro(macro_for_each_gen)
  apply_macro_for_each_platform()
endmacro()
apply_macro_for_each_gen("TESTED")

set_property(TARGET run_ocloc_tests PROPERTY FOLDER ${CLOC_FOLDER})

if(WIN32)
  add_custom_command(
    TARGET run_ocloc_tests
    POST_BUILD
    COMMAND echo deleting offline compiler files and directories...
    COMMAND ${CMAKE_COMMAND} -E remove ${TargetDir}/${CLOC_DEFAULT_DEVICE}/copybuffer_${CLOC_DEFAULT_DEVICE}.bc
    COMMAND ${CMAKE_COMMAND} -E remove ${TargetDir}/${CLOC_DEFAULT_DEVICE}/copybuffer_${CLOC_DEFAULT_DEVICE}.gen
    COMMAND ${CMAKE_COMMAND} -E remove ${TargetDir}/${CLOC_DEFAULT_DEVICE}/copybuffer_${CLOC_DEFAULT_DEVICE}.bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TargetDir}/${CLOC_DEFAULT_DEVICE}/offline_compiler_test
    COMMAND ${run_tests_cmd}
    WORKING_DIRECTORY ${TargetDir}
  )
else()
  add_custom_command(
    TARGET run_ocloc_tests
    POST_BUILD
    COMMAND echo deleting offline compiler files and directories...
    COMMAND ${CMAKE_COMMAND} -E remove ${TargetDir}/${CLOC_DEFAULT_DEVICE}/*.bc
    COMMAND ${CMAKE_COMMAND} -E remove ${TargetDir}/${CLOC_DEFAULT_DEVICE}/*.gen
    COMMAND ${CMAKE_COMMAND} -E remove ${TargetDir}/${CLOC_DEFAULT_DEVICE}/*.ll
    COMMAND ${CMAKE_COMMAND} -E remove ${TargetDir}/${CLOC_DEFAULT_DEVICE}/*.bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${TargetDir}/offline_compiler_test"
    COMMAND ${run_tests_cmd}
    WORKING_DIRECTORY ${TargetDir}
  )
endif()

add_subdirectories()
create_project_source_tree(ocloc_tests)
