cmake_minimum_required(VERSION 3.6)

add_compile_options(-std=c++14)

file(GLOB app_src_files "src/main/jni/main.cpp" "src/main/jni/webgl.cc" "src/main/jni/bindings.cc" )

add_library( nodebinding SHARED ${app_src_files} )

include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/include/node )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/include/native_app_glue )

add_library( nodelib SHARED IMPORTED )
set_target_properties( nodelib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libnode.so )

find_library( log-lib log )
find_library( android-lib android )
find_library( egl-lib EGL )
find_library( glesv3-lib GLESv3 )
target_link_libraries( nodebinding ${log-lib} ${android-lib} ${egl-lib} ${glesv3-lib} nodelib )
