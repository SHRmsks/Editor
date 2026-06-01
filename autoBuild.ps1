Remove-Item -Force -ErrorAction SilentlyContinue ".\client\public\clib.wasm"
cmake -S Clib -B Clib/build -DCMAKE_C_COMPILER="C:\Program Files\LLVM\bin\clang.exe" -DCMAKE_SYSTEM_NAME=Generic
cmake --build Clib/build