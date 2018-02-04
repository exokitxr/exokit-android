cmake_minimum_required(VERSION 3.6)

add_compile_options(-std=c++14 -pthread)

file(GLOB app_src_files
  "src/main/jni/main.cpp"
  "src/main/jni/deps/bindings/src/*.cc"
  "src/main/jni/deps/canvas/src/*.cpp"
  "src/main/jni/deps/nanosvg/src/*.cpp"
  "src/main/jni/deps/canvascontext/src/*.cc"
  "src/main/jni/deps/webglcontext/src/*.cc"
  "src/main/jni/deps/platform/android/src/*.cpp"
)

add_library( nodebinding SHARED ${app_src_files} )

include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/node )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/native_app_glue )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/util/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/bindings/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/canvas/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/nanosvg/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/canvascontext/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/webglcontext/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/jni/deps/platform/android/include )

add_library( nodelib SHARED IMPORTED )
set_target_properties( nodelib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libnode.so )

find_library( log-lib log )
find_library( android-lib android )
find_library( egl-lib EGL )
find_library( glesv3-lib GLESv3 )
find_library( jnigraphics-lib jnigraphics )
target_link_libraries( nodebinding ${log-lib} ${android-lib} ${egl-lib} ${glesv3-lib} ${jnigraphics-lib} nodelib )
