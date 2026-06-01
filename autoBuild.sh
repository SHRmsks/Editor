#!/bin/bash
# define the function that check and install dependencies based on the OS
install_dependencies() {
    echo "⚠️Missing required build tools. Attempting automatic installation..."
    OS="$(uname -s)"
    case "${OS}" in
        Linux*)
        # if command -v returns 0, it means the command exists
            if command -v apt-get &> /dev/null; then
                echo "🐧 Debian/Ubuntu/WSL detected. Installing via apt..."
                sudo apt-get update
                sudo apt-get install -y cmake clang lld
            elif command -v dnf &> /dev/null; then
                echo "🐧 Fedora/RHEL detected. Installing via dnf..."
                sudo dnf install -y cmake clang lld
            else
                echo "❌ Could not find a supported package manager. Please install cmake, clang, and lld manually."
                exit 1
            fi
            ;;
        Darwin*)
            if command -v brew &> /dev/null; then
                echo "🍎 macOS detected. Installing via Homebrew..."
                brew install cmake llvm
            else
                echo "❌ Homebrew is not installed! Please install Homebrew (https://brew.sh) and try again."
                exit 1
            fi
            ;;
        CYGWIN*|MINGW*|MSYS*|MINGW32*)
            echo "🪟 Windows (Git Bash/MinGW) detected."
            if command -v winget &> /dev/null; then
                echo "Installing CMake and LLVM via winget..."
                winget install -e --id Kitware.CMake
                winget install -e --id LLVM.LLVM
                echo "✅ Installation complete. PLEASE RESTART YOUR TERMINAL and run this script again."
                exit 1
            else
                echo "❌ winget not found. Please install CMake and LLVM manually."
                exit 1
            fi
            ;;
        *)
            echo "❌ Unsupported operating system: ${OS}. Please install manually."
            exit 1
            ;;
    esac
}

if ! command -v cmake &> /dev/null || ! command -v clang &> /dev/null; then
    install_dependencies
fi

CLANG_PATH=""
if [[ "$OSTYPE" == "darwin"* ]]; then
    if [ -f "/opt/homebrew/opt/llvm/bin/clang" ]; then
        CLANG_PATH="/opt/homebrew/opt/llvm/bin/clang" # Apple Silicon
    elif [ -f "/usr/local/opt/llvm/bin/clang" ]; then
        CLANG_PATH="/usr/local/opt/llvm/bin/clang" # Intel Mac
    else
        install_dependencies
        if [ -f "/opt/homebrew/opt/llvm/bin/clang" ]; then
            CLANG_PATH="/opt/homebrew/opt/llvm/bin/clang"
        elif [ -f "/usr/local/opt/llvm/bin/clang" ]; then
            CLANG_PATH="/usr/local/opt/llvm/bin/clang"
        else
            echo "❌ Critical Error: Installed LLVM but cannot find it in Homebrew paths."
            exit 1
        fi
    fi
else
    # Linux, WSL, and Windows Git Bash just use 'clang' in the PATH
    if ! command -v clang &> /dev/null; then
        install_dependencies
    fi
    CLANG_PATH="clang"
fi

echo "🧹 Cleaning previous build cache..."
rm -rf Clib/build

echo "🔨 Compiling C to WebAssembly..."
cmake -S Clib -B Clib/build -DCMAKE_C_COMPILER="$CLANG_PATH"
if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed."
    exit 1
fi

cmake --build Clib/build

if [ $? -eq 0 ]; then
    echo "🚀 Build successful! clib.wasm is ready in client/public/"
else
    echo "❌ Build failed. Check the errors above."
    exit 1
fi