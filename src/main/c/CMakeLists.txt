project(bittuplayer)

set(CMAKE_C_FLAGS_DEBUG_INIT "-g -O0")

add_library(bittuplayer SHARED bittu_jni_BittuplayerJNI.c)
target_include_directories(bittuplayer PRIVATE
    "$ENV{JAVA_HOME}/include"
    "$ENV{JAVA_HOME}/include/linux")
target_link_libraries(bittuplayer PRIVATE avformat avutil)
