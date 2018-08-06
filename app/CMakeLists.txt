cmake_minimum_required(VERSION 3.6)

add_compile_options(-pthread )



file(GLOB app_src_files
  "src/main/main.cpp"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/bindings/src/*.cc"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/util/src/*.cc"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/canvas/src/*.cpp"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/nanosvg/src/*.cpp"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/canvascontext/src/*.cc"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/webglcontext/src/*.cc"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/webaudiocontext/src/*.cpp"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/videocontext/src/*.cpp"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/videocontext/src/linux/*.cpp"
  "src/main/assets/node/node_modules/exokit/deps/exokit-bindings/glfw/src/*.cc"
)
add_library( nodebinding SHARED ${app_src_files} )

include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/utf8 )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/native_app_glue )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/util/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/bindings/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/canvas/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/nanosvg/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/canvascontext/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/webglcontext/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/webaudiocontext/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/videocontext/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/exokit/deps/exokit-bindings/glfw/include )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/other_deps/context/android )





add_library( node SHARED IMPORTED )
set_target_properties( node PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/arm64-v8a/libnode.so )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/other_deps/node )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/other_deps/v8/include )


add_library( GLEW STATIC IMPORTED )
set_target_properties( GLEW PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-graphics-deps/lib2/arm64/glew/libGLEW.a )
add_library( glfw3 STATIC IMPORTED )
set_target_properties( glfw3 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-graphics-deps/lib2/arm64/glfw/libglfw3.a )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-graphics-deps/include )


add_library( png16 STATIC IMPORTED )
set_target_properties( png16 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-canvas-deps/lib2/arm64/libpng16.a )
add_library( skia STATIC IMPORTED )
set_target_properties( skia PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-canvas-deps/lib2/arm64/libskia.a )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-canvas-deps/include/core )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-canvas-deps/include/config )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-canvas-deps/include/gpu )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-canvas-deps/include/effects )


add_library( LabSound STATIC IMPORTED )
set_target_properties( LabSound PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-audio-deps/lib2/arm64/libLabSound.a )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-audio-deps/include )




add_library( avcodec STATIC IMPORTED )
set_target_properties( avcodec PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-video-deps/lib2/arm64/libavcodec.a )
add_library( avdevice STATIC IMPORTED )
set_target_properties( avdevice PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-video-deps/lib2/arm64/libavdevice.a )
add_library( avfilter STATIC IMPORTED )
set_target_properties( avfilter PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-video-deps/lib2/arm64/libavfilter.a )
add_library( avformat STATIC IMPORTED )
set_target_properties( avformat PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-video-deps/lib2/arm64/libavformat.a )
add_library( avutil STATIC IMPORTED )
set_target_properties( avutil PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-video-deps/lib2/arm64/libavutil.a )
add_library( swresample STATIC IMPORTED )
set_target_properties( swresample PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-video-deps/lib2/arm64/libswresample.a )
add_library( swscale STATIC IMPORTED )
set_target_properties( swscale PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-video-deps/lib2/arm64/libswscale.a )
include_directories( SYSTEM ${CMAKE_SOURCE_DIR}/src/main/assets/node/node_modules/native-video-deps/include )





find_library( log-lib log )
find_library( android-lib android )
find_library( egl-lib EGL )
find_library( glesv2-lib GLESv2 )
find_library( glesv3-lib GLESv3 )
find_library( jnigraphics-lib jnigraphics )
target_link_libraries( nodebinding
                       ${log-lib}
                       ${android-lib}
                       ${glesv2-lib}
                       ${glesv3-lib}
                       ${egl-lib}
                       ${jnigraphics-lib}
                       node
                       png16
                       skia
                       LabSound
                       avformat
                       avcodec
                       avutil
                       avdevice
                       swscale
                       swresample
                       )
