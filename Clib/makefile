CC = /opt/homebrew/opt/llvm/bin/clang
CFLAGS = --target=wasm32 -O2 -flto -nostdlib -Wall -Wextra
LDFLAGS = -fuse-ld=/opt/homebrew/opt/lld/bin/wasm-ld -Wl,--no-entry -Wl,--allow-undefined -Wl,--export=memory,--export-dynamic
SRC = $(wildcard *.c)
OUT = ../client/public/clib.wasm

all: $(OUT)

$(OUT): $(SRC)
	@echo "Compiling C to WebAsm..." 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	@echo "Build successful: $@"
	
clean: 
	@echo "Cleaning up..."
	rm -rf $(OUT)
	@echo "Clean complete."