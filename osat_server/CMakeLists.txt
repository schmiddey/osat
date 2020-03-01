set(project_name osat_server)

project(${project_name})

cmake_minimum_required (VERSION 2.6)

#set compiler flags
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O2 -Wall -std=c++17")
#set Variables



find_package(Boost COMPONENTS date_time filesystem graph iostreams 
               math_c99 mpi prg_exec_monitor program_options
               regex serialization signals system thread wave REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})



# find_package(Boost REQUIRED COMPONENTS)
# FIND_PACKAGE(OpenCV 2 REQUIRED )

# include_directories(${Boost_INCLUDE_DIR}
#                     )

# link_directories(${Boost_LIBRARY_DIR} )

#grab files
file(GLOB src_server "src/*.cpp")

file(GLOB src_boost "src/boost/*.cpp")


#add executable
#udp boost class test/tool
add_executable(test_udp_rcv   test/test_udp_rcv.cpp)
add_executable(test_udp_send  test/test_udp_send.cpp)
#tcp boost examples (no class)
add_executable(test_tcp_srv_basic test/test_api/test_tcp_srv_basic.cpp)
add_executable(test_tcp_cl_basic  test/test_api/test_tcp_cl_basic.cpp)
#tcp boost example with own tcp class
add_executable(test_tcp_srv test/test_tcp_srv.cpp)
add_executable(test_tcp_cl  test/test_tcp_cl.cpp)



target_link_libraries(test_udp_rcv  ${Boost_LIBRARIES})
target_link_libraries(test_udp_send ${Boost_LIBRARIES})

target_link_libraries(test_tcp_srv_basic  ${Boost_LIBRARIES})
target_link_libraries(test_tcp_cl_basic   ${Boost_LIBRARIES})

target_link_libraries(test_tcp_srv  ${Boost_LIBRARIES})
target_link_libraries(test_tcp_cl   ${Boost_LIBRARIES})

#link executable
# target_link_libraries(template ${Boost_LIBRARIES}
#                                ${OpenCV_LIBRARIES}
#                                libxxx
#                                )


#install
#install(TARGETS template
#        RUNTIME DESTINATION bin
#        )

#install(FILES config/template.xml DESTINATION /etc/template/)
