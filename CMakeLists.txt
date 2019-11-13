cmake_minimum_required(VERSION 3.0)
project(biome)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_STANDARD 17)
set(CURSES_NEED_NCURSES TRUE)

file(GLOB BIOME_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.h")
add_executable(biome "${BIOME_SOURCES}")

find_package(Curses REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(biome "${CURSES_LIBRARIES}" "${CMAKE_THREAD_LIBS_INIT}")
message("Curses Libraries are ${CURSES_LIBRARIES}.")


#install(TARGETS biome RUNTIME DESTINATION bin)

#add_executable(cursestest "${CMAKE_SOURCE_DIR}/tests/cursestest.cpp")
#target_link_libraries(cursestest "${CURSES_LIBRARIES}")
