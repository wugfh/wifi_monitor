cmake_minimum_required(VERSION 3.10)

project(mini_mlo)


if(NOT UNIX)
	message(FATAL_ERROR "os must be unix-like syetem, such as linux, freebsd")
endif()

find_package(PythonLibs REQUIRED)
if(PYTHONLIBS_FOUND)
	message(STATUS "python version " ${PYTHONLIBS_VERSION_STRING})
else()
	message(STATUS "need python3")
endif()

include_directories(include ${PYTHON_INCLUDE_DIRS})
link_libraries(${PYTHON_LIBRARIES})
add_definitions(-D_DEFAULT_SOURCE 
				-DSNIFF_80211)
add_compile_options(-O0 -g)
SET(LIBRARIES
	pthread
	m
)

#compile mlo_udp

find_package(PkgConfig REQUIRED)
pkg_check_modules(mlo_lib IMPORTED_TARGET 
	libnl-3.0 
	libnl-route-3.0 
	libnl-genl-3.0
	libnl-cli-3.0
	libpcap
	libxdp
	libbpf
)
if(NOT mlo_lib_FOUND)
message(STATUS intstall package)
execute_process(COMMAND sudo apt install libnl-3-dev)
execute_process(COMMAND sudo apt install libnl-route-3-dev)
execute_process(COMMAND sudo apt install libpcap-dev)
execute_process(COMMAND sudo apt install libnl-genl-3-dev)
execute_process(COMMAND sudo apt install libnl-cli-3-dev)
execute_process(COMMAND sudo apt install libxdp-dev)
execute_process(COMMAND sudo apt install libbpf-dev)
message(FATAL_ERROR "install done, please retry cmake")
endif()

aux_source_directory(sta sta_src)
aux_source_directory(src all_src)
aux_source_directory(ap ap_src)
aux_source_directory(mon mon_src)
add_library(mymlo STATIC ${all_src})
add_executable(sta_send ${sta_src})
add_executable(ap_recv  ${ap_src})
add_executable(chan_mon  ${mon_src})
target_link_libraries(sta_send PkgConfig::mlo_lib mymlo)
target_compile_options(
    sta_send
    PRIVATE
)
target_link_libraries(ap_recv PkgConfig::mlo_lib mymlo)
target_compile_definitions(ap_recv PRIVATE AP_RECV)
target_compile_options(
    ap_recv
    PRIVATE       
)

target_link_libraries(chan_mon PkgConfig::mlo_lib mymlo)
target_compile_options(
    chan_mon
    PRIVATE       
)