cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0) # для совместимости с библиотеками pcap++ скомпилированными для centos 7

add_executable(main main.cpp src/JA3_parser.cpp src/JA3_Config.cpp)

target_include_directories(
    main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/pcappp/header"
    "${CMAKE_CURRENT_SOURCE_DIR}/headers"
)


# так как в centos 7 устаревшие версии libm и libstdc++, то некоторые либы решил скомпилировать статически
target_link_libraries(
    main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/pcappp/libPcap++.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/pcappp/libPacket++.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/pcappp/libCommon++.a"
	"libpcap.a"
	"libm-2.31.a"
	"libc.a"
	pthread
    pcap
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
