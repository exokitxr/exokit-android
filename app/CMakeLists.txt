cmake_minimum_required(VERSION 3.6)

add_compile_options(-std=c++11)
# add_definitions(-DANDROID_TOOLCHAIN=clang -DANDROID_STL=c++_shared -DANDROID_PLATFORM=android-25)

file(GLOB app_src_files "src/main/jni/main.cpp" )

add_library( nodebinding SHARED ${app_src_files} )

include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/include/node )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/include/native_app_glue )

add_library( nodelib SHARED IMPORTED )

set_target_properties( nodelib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libnode.so )

find_library( log-lib log )
find_library( android-lib android )
find_library( egl-lib EGL )
find_library( glesv2-lib GLESv2 )
target_link_libraries( nodebinding ${log-lib} ${android-lib} ${egl-lib} ${glesv2-lib} nodelib )
