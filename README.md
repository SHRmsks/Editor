## Online Rich Text Editor
*** 
### Prerequisite:
1. Make sure you have downloaded the browser that supports **WebGL 2.0**.
    - FireFox: Version 51+
    - Chrome: Version 56+
    - Microsoft Edge: Version 79+
    - Safari: Version 15+
2. If you haven't, please download **CMake**, **LLVM Clang** Toolchain, **Node & npm**
3. Modify and run autoBuild.sh | autoBuild.ps1 
    * change the `-DCMAKE_C_COMPILER` flag to the actual **PATH** in your machine
    eg. MacOS using Homebrew will be: `/opt/homebrew/opt/llvm/bin/clang`, Linux will be similar to `/usr/bin/clang` and Windows will be similar to `"C:\Program Files\LLVM\bin\clang.exe"`
    * If you are not sure, type `which clang` to acquire the absolute path after installation. 
    * Once you can see client/public/Clib.wasm you have successfully built!
4. cd **client** and run `npm install && npm run dev` to start developing!
*** 
### Core Architectures
1. This project is adopting **Piece Table** and **Red Black Tree** as its major data structures for managing the text editing.
 