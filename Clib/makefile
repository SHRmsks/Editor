CC = /opt/homebrew/opt/llvm/bin/clang
CFLAGS = --target=wasm32 -O3 -flto -nostdlib -Wall -Wextra
LDFLAGS = -fuse-ld=/opt/homebrew/opt/lld/bin/wasm-ld -Wl,--no-entry -Wl,--allow-undefined
SRC = $(wildcard *.c)
OUT = clib.wasm

all: $(OUT)

$(OUT): $(SRC)
	@echo "Compiling C Kernel to WebAsm..." 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Build successful: $@"
	
clean: 
	@echo "Cleaning up..."
	rm -rf $(OUT)
	@echo "Clean complete."