#!bin/bash

# Turn off to build to web
# BUILD_WEB=false

defines=""
includes="-Ithird_party/include -Iassets/shaders"

# PC
libs="-Lthird_party/lib -lsdl3 -lopengl32 -luser32 -lshell32 -lgdi32 -lmsvcrt -lwinmm -nostdlib"
warnings="-Wno-writable-strings -Wno-c99-designator -Wno-nullability-completeness"
clang++ -std=c++20 $warnings -g main.cpp -o main.exe $defines $includes $libs "-DSDL3"
echo Succesfully build PC Version.

# also my games run like shit on the web in debug mode. they run fine with -O3, -sASSERTIONS=0, -sSAFE_HEAP=0, no sanitizer

# WEB
if $BUILD_WEB ; then
  inclWeb="-I. -I ./third_party/include"
  shellHTML="--shell-file ./third_party/include/shell.html"
  webOptions="-s TOTAL_STACK=64MB -s INITIAL_MEMORY=128MB -s ASSERTIONS -s USE_SDL=3 -s FULL_ES3=1 -DWEB_BUILD"
  libsWeb="third_party/lib/libSDL3.a third_party/lib/libSDL3_image.a third_party/lib/libSDL3_ttf.a -lm  third_party/lib/libpng16.a third_party/lib/libzlibstatic.a"
  preload="--preload-file assets"
  warnings="-Wno-c99-designator -Wno-writable-strings -Wno-nullability-completeness -Wno-writable-strings -Wno-missing-braces"
  C:/emsdk/upstream/emscripten/em++.bat -o ./index.html main.cpp -Os -Wall $warnings $preload $webOptions $inclWeb $shellHTML $libsWeb 

  testOutput="test"
  cp index.* $testOutput
fi


# C:\emsdk\upstream\emscripten\em++.bat -O3 -DNDEBUG -sUSE_SDL=0 -sALLOW_MEMORY_GROWTH=1 --shell-file \
# C:/Users/User/Development/sdl3-sample/shell.html --preload-file C:/Users/User/Development/sdl3-sample/assets@/assets \
# CMakeFiles/sdl3-sample.dir/src/main.cpp.o -o index.html  _deps/sdl3-build/libSDL3.a  _deps/sdl3_image-build/libSDL3_image.a  \
# -lm  _deps/sdl3_image-build/external/libpng-build/libpng16.a  _deps/sdl3_image-build/external/zlib-build/libzlibstatic.a

