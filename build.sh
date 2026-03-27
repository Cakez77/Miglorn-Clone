defines=""
includes="-Ithird_party/include -Iassets/shaders"

# Raylib
# libs="-Lthird_party/lib -lraylib -luser32 -lshell32 -lgdi32 -lmsvcrt -lwinmm -nostdlib"
# clang++ -std=c++20 -Wno-c99-designator -g game.cpp -o game.exe $defines $includes $libs "-DRAYLIB"
# WEB
libsWeb="-L ./third_party/lib"
inclWeb="-I. -I ./third_party/include"
shellHTML="--shell-file ./third_party/include/shell.html"
webOptions="-Wno-c99-designator -s ASYNCIFY -s TOTAL_STACK=64MB -s INITIAL_MEMORY=128MB -s ASSERTIONS -s USE_GLFW=3 -DPLATFORM_WEB -DRAYLIB"
C:/emsdk/upstream/emscripten/emcc.bat -o ./index.html game.cpp -Os -Wall --preload-file assets/textures $libsWeb $inclWeb $shellHTML $webOptions -lraylib
# emcc 

# SDL3
# libs="-Lthird_party/lib -lsdl3 -luser32 -lshell32 -lgdi32 -lmsvcrt -lwinmm -nostdlib"
# clang++ -std=c++20 -Wno-c99-designator -g game.cpp -o game.exe $defines $includes $libs "-DSDL3"
# WEB
# libsWeb="-L ./third_party/lib"
# inclWeb="-I. -I ./third_party/include"
# shellHTML="--shell-file ./third_party/include/shell.html"
# webOptions="-s ASYNCIFY -s TOTAL_STACK=64MB -s INITIAL_MEMORY=128MB -s ASSERTIONS -s USE_SDL=3 -DPLATFORM_WEB -DSDL3"
# C:/emsdk/upstream/emscripten/emcc.bat -o ./index.html game.cpp -Os -Wall $libsWeb $inclWeb $shellHTML $webOptions -lsdl3

# WEB
# emcc -o ./index.html game.cpp -Os -Wall -L ./third_party/lib -I. -I ./third_party/include -s USE_GLFW=3 -s ASYNCIFY --shell-file ./third_party/include/shell.html -s TOTAL_STACK=64MB -s INITIAL_MEMORY=128MB -s ASSERTIONS -DPLATFORM_WEB 

# OLD
# "cmake --build build"
# "zig c++ -Lthird_party/lib -lraylib -luser32 -lshell32 -lgdi32 -lmsvcrt -lwinmm -Ithird_party/include -o game.exe game.cpp"
# "clang game.c -o game.exe -g -Lthird_party/lib -lraylib -luser32 -lshell32 -lgdi32 -lmsvcrt -lwinmm -Ithird_party/include"