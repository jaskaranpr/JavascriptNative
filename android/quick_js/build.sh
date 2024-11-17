#!/bin/bash

# Exit on any error
set -e

# Set NDK path
export ANDROID_NDK="/Users/jaskaran.s/Library/Android/sdk/ndk/25.2.9519653"

echo "Using Android NDK at: $ANDROID_NDK"

# Verify NDK exists
if [ ! -d "$ANDROID_NDK" ]; then
    echo "Error: Android NDK not found at $ANDROID_NDK"
    exit 1
fi

# Create working directory
WORK_DIR="quickjs_build"
mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

# Clone QuickJS if not already cloned
if [ ! -d "quickjs" ]; then
    echo "Cloning QuickJS..."
    git clone https://github.com/bellard/quickjs.git
fi

cd quickjs

# Create the Android Makefile
echo "Creating Android Makefile..."
cat > Makefile.android << 'EOL'
# Android arm64 configuration
PREFIX = /usr/local

# NDK paths - these will be set from environment variables
CC = $(ANDROID_NDK)/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android24-clang
AR = $(ANDROID_NDK)/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-ar
STRIP = $(ANDROID_NDK)/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-strip

# Compiler flags
CFLAGS = -Os -fPIC -target aarch64-linux-android24
CFLAGS += -g -Wall -Wextra -Wno-sign-compare -Wno-missing-field-initializers
CFLAGS += -Wundef -Wuninitialized -Wunused -Wno-unused-parameter
CFLAGS += -Wwrite-strings -Wchar-subscripts -funsigned-char -MMD
CFLAGS += -fwrapv -DANDROID
CFLAGS += -D_GNU_SOURCE -DCONFIG_VERSION=\"2024-02-14\"
CFLAGS += -DCONFIG_BIGNUM

LDFLAGS = -shared

# Source files
SRCS = quickjs.c libunicode.c libregexp.c libbf.c cutils.c
OBJS = $(SRCS:.c=.o)

# Targets
all: libquickjs.a

libquickjs.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.a *.d *~
EOL

# Clean any previous builds
echo "Cleaning previous build..."
make -f Makefile.android clean

# Build
echo "Building QuickJS..."
make -f Makefile.android

# Create directories for output
echo "Creating output directories..."
mkdir -p ../../android/app/src/main/jniLibs/arm64-v8a
mkdir -p ../../android/app/src/main/cpp/quickjs/include

# Copy the built library
echo "Copying built library..."
cp libquickjs.a ../../android/app/src/main/jniLibs/arm64-v8a/

# Copy necessary headers
echo "Copying headers..."
cp quickjs.h ../../android/app/src/main/cpp/quickjs/include/
cp quickjs-libc.h ../../android/app/src/main/cpp/quickjs/include/

echo "Build complete!"
