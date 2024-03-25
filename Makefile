TARGET_DIR := godot_markdown/target
LIB_DIR := addons/godot_markdown/lib
ZIP_FILE := godot_markdown.zip

linux := x86_64-unknown-linux-gnu aarch64-unknown-linux-gnu
windows := x86_64-pc-windows-gnu
android := aarch64-linux-android armv7-linux-androideabi x86_64-linux-android
web := wasm32-unknown-emscripten
macos := aarch64-apple-darwin x86_64-apple-darwin
ios := aarch64-apple-ios
platforms := $(linux) $(windows) $(android)

.PHONY: all clean

all: $(platforms) $(ZIP_FILE)

$(platforms):
	@echo Building $@
	cd godot_markdown; cross build --target $@
	cd godot_markdown; cross build --target $@ --release
	mkdir -p $(LIB_DIR)/$@/debug $(LIB_DIR)/$@/release
	$(eval lib := $(if $(findstring windows,$@), godot_markdown.dll, libgodot_markdown.so))
	cp $(TARGET_DIR)/$@/debug/$(lib) $(LIB_DIR)/$@/debug/
	cp $(TARGET_DIR)/$@/release/$(lib) $(LIB_DIR)/$@/release/

$(ZIP_FILE):
	zip $@ addons -x "*/.*"

clean:
	rm -rf $(LIB_DIR)
	rm -f $(ZIP_FILE)
