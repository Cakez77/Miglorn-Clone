#!bin/bash

# Turn off to build to web
BUILD_WEB=false

# PC Data
defines=""
includes="-Ithird_party/include -Iassets/shaders"
outputFile=main.exe
libs="-Lthird_party/lib -lfreetype -lsdl3 -lopengl32 -luser32 -lshell32 -lgdi32 -lmsvcrt -lwinmm -nostdlib"
warnings="-Wno-writable-strings -Wno-c99-designator -Wno-nullability-completeness"

# Precompiled Lib header
# time clang++ $defines $optimize -c -o "src/lib.h.pch" "src/lib.h" -std=c++20 -Ithird_party $includes $warnings

# WEB
if $BUILD_WEB ; then
  # also my games run like shit on the web in debug mode. 
  # They run fine with -O3, -sASSERTIONS=0, -sSAFE_HEAP=0, no sanitizer
  inclWeb="-I. -I ./third_party/include"
  shellHTML="--shell-file ./third_party/include/shell.html"
  webOptions="-s TOTAL_STACK=64MB -s INITIAL_MEMORY=128MB -s ASSERTIONS -s USE_SDL=3 -s FULL_ES3=1 -DWEB_BUILD"
  libsWeb="third_party/lib/libSDL3.a third_party/lib/libSDL3_image.a third_party/lib/libSDL3_ttf.a -lm  third_party/lib/libpng16.a third_party/lib/libzlibstatic.a"
  preload="--preload-file assets"
  warnings="-Wno-c99-designator -Wno-writable-strings -Wno-nullability-completeness -Wno-writable-strings -Wno-missing-braces"
  C:/emsdk/upstream/emscripten/em++.bat -o web/index.html src/main.cpp -fchar8_t -Os -Wall $warnings $preload $webOptions $inclWeb $shellHTML $libsWeb 
else
  timestamp=$(date +%s)
  echo "Building game.dll..."

  rm -f game_* # Remove old game_* files
  time clang++ -Wl,/ignore:4099 -include-pch "src/lib.h.pch" -std=c++20 $warnings -g src/game.cpp -shared -o game_$timestamp.dll $defines $includes $libs "-DSDL3"
  mv game_$timestamp.dll game.dll
  echo Succesfully build game.dll
fi

# Build Engine if not running
processRunning=$(tasklist | grep $outputFile)
if [ -z "$processRunning" ]; then
    echo "Engine not running, building main..."
    time clang++ -Wl,/ignore:4099 -include-pch "src/lib.h.pch" -std=c++20 $warnings -g src/main.cpp -o $outputFile $defines $includes $libs -lDbgHelp "-DSDL3"
else
    echo "Engine running, not building!"
fi

# C:\emsdk\upstream\emscripten\em++.bat -O3 -DNDEBUG -sUSE_SDL=0 -sALLOW_MEMORY_GROWTH=1 --shell-file \
# C:/Users/User/Development/sdl3-sample/shell.html --preload-file C:/Users/User/Development/sdl3-sample/assets@/assets \
# CMakeFiles/sdl3-sample.dir/src/main.cpp.o -o index.html  _deps/sdl3-build/libSDL3.a  _deps/sdl3_image-build/libSDL3_image.a  \
# -lm  _deps/sdl3_image-build/external/libpng-build/libpng16.a  _deps/sdl3_image-build/external/zlib-build/libzlibstatic.a

