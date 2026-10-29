#!/usr/bin/env bash
#
# Build a self-contained AppImage of XBlast TNT.
#
# The script builds the game from a clean copy of the source tree (the
# working copy is left untouched), stages an AppDir and packs it with
# appimagetool.  appimagetool is downloaded automatically if it is not
# already installed.
#
# Usage:
#   ./build-appimage.sh [options]
#
# Options:
#   -o, --output DIR     Where to put the .AppImage (default: ./dist)
#   -b, --build-dir DIR  Scratch/build directory (default: ./build/appimage)
#   -j, --jobs N         Parallel build jobs (default: number of CPUs)
#       --no-strip       Do not strip the binaries
#   -h, --help           Show this help
#
# Requirements:
#   gcc/clang + make, tar, autoconf/automake/autoheader, the SDL3
#   development files (pkg-config modules sdl3, sdl3-image, sdl3-ttf and
#   sdl3-mixer), and ImageMagick ("magick" or "convert") to create the
#   application icon.
#   A Vera or DejaVu TrueType font is bundled for the in-game text.
#
set -euo pipefail
umask 022

APP_ID="xblast-tnt"
APP_NAME="XBlast TNT"
DESKTOP_NAME="${APP_ID}.desktop"

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
OUT_DIR="${SCRIPT_DIR}/dist"
BUILD_DIR="${SCRIPT_DIR}/build/appimage"
JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
STRIP=1

usage() {
    cat <<'EOF'
Build a self-contained AppImage of XBlast TNT.

The script builds the game from a clean copy of the source tree (the
working copy is left untouched), stages an AppDir and packs it with
appimagetool.  appimagetool is downloaded automatically if it is not
already installed.

Usage:
  ./build-appimage.sh [options]

Options:
  -o, --output DIR     Where to put the .AppImage (default: ./dist)
  -b, --build-dir DIR  Scratch/build directory (default: ./build/appimage)
  -j, --jobs N         Parallel build jobs (default: number of CPUs)
      --no-strip       Do not strip the binaries
  -h, --help           Show this help

Requirements:
  gcc/clang + make, tar, autoconf/automake/autoheader, the SDL3 development
  files (pkg-config modules sdl3, sdl3-image, sdl3-ttf and sdl3-mixer), and
  ImageMagick ("magick" or "convert") to create the application icon.
  A Vera or DejaVu TrueType font is bundled for the in-game text.
EOF
}

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

log() {
    printf '==> %s\n' "$*"
}

while [ $# -gt 0 ]; do
    case "$1" in
        -o|--output)   OUT_DIR=$2; shift 2 ;;
        -b|--build-dir) BUILD_DIR=$2; shift 2 ;;
        -j|--jobs)     JOBS=$2; shift 2 ;;
        --no-strip)    STRIP=0; shift ;;
        -h|--help)     usage; exit 0 ;;
        *)             die "unknown option: $1 (try --help)" ;;
    esac
done

case "$OUT_DIR" in /*) ;; *) OUT_DIR="${SCRIPT_DIR}/${OUT_DIR}" ;; esac
case "$BUILD_DIR" in /*) ;; *) BUILD_DIR="${SCRIPT_DIR}/${BUILD_DIR}" ;; esac

SRC_DIR="$BUILD_DIR/src"
APPDIR="$BUILD_DIR/AppDir"

# ---------------------------------------------------------------------------
# Checks
# ---------------------------------------------------------------------------
for tool in tar make; do
    command -v "$tool" >/dev/null 2>&1 || die "'$tool' is required"
done
if ! command -v cc >/dev/null 2>&1 && ! command -v gcc >/dev/null 2>&1; then
    die "a C compiler (cc or gcc) is required"
fi
if command -v magick >/dev/null 2>&1; then
    IM=magick
elif command -v convert >/dev/null 2>&1; then
    IM=convert
else
    die "ImageMagick (magick or convert) is required to create the icon"
fi

# Version number, e.g. 2.10.4
VERSION=$(sed -n \
    -e 's/^#define VERSION_MAJOR[[:space:]]*\([0-9][0-9]*\).*/\1/p' \
    -e 's/^#define VERSION_MINOR[[:space:]]*\([0-9][0-9]*\).*/\1/p' \
    -e 's/^#define VERSION_PATCH[[:space:]]*\([0-9][0-9]*\).*/\1/p' \
    "$SCRIPT_DIR/version.h" | paste -sd. -)
[ -n "$VERSION" ] || die "could not determine version from version.h"

case "$(uname -m)" in
    x86_64|amd64)  ARCH="x86_64" ;;
    aarch64|arm64) ARCH="aarch64" ;;
    armv7l|armv7)  ARCH="armhf" ;;
    i?86)          ARCH="i686" ;;
    *)             ARCH=$(uname -m) ;;
esac

OUTPUT="$OUT_DIR/${APP_ID}-${VERSION}-${ARCH}.AppImage"

log "XBlast TNT ${VERSION}, target ${ARCH}"
log "Build directory: $BUILD_DIR"
log "Output:          $OUTPUT"

# ---------------------------------------------------------------------------
# Clean copy of the sources
#
# Building in-tree is avoided because the autotools versions used by this
# project refuse out-of-tree builds of an already configured tree.
# ---------------------------------------------------------------------------
log "Copying sources"
rm -rf -- "$BUILD_DIR"
mkdir -p -- "$SRC_DIR" "$OUT_DIR"
tar -C "$SCRIPT_DIR" -cf - \
    --exclude='./build' \
    --exclude='./dist' \
    --exclude='./.git' \
    --exclude='./autom4te.cache' \
    --exclude='./.deps' \
    --exclude='*.o' \
    --exclude='*.a' \
    --exclude='*.AppImage' \
    --exclude='./config.cache' \
    --exclude='./config.log' \
    --exclude='./config.status' \
    --exclude='./config.h' \
    --exclude='./stamp-h1' \
    --exclude='./Makefile' \
    --exclude='./po/Makefile' \
    --exclude='./xblast' \
    --exclude='./xbsndsrv' \
    --exclude='./configure' \
    --exclude='./aclocal.m4' \
    --exclude='./config.h.in' \
    --exclude='./Makefile.in' \
    --exclude='./po/Makefile.in' \
    --exclude='*~' \
    . | tar -C "$SRC_DIR" -xf -

# ---------------------------------------------------------------------------
# Configure and build
# ---------------------------------------------------------------------------
cd "$SRC_DIR"

if [ ! -x ./configure ]; then
    log "Generating configure script"
    ./autogen.sh
fi

# pkg-config does not always search the system's libdir (e.g. /usr/lib64),
# so look for the SDL3 modules there as a fallback.
if ! pkg-config --exists sdl3 2>/dev/null; then
    for pc_dir in /usr/lib64/pkgconfig /usr/local/lib64/pkgconfig \
                  /usr/lib/pkgconfig /usr/local/lib/pkgconfig; do
        if [ -f "$pc_dir/sdl3.pc" ]; then
            export PKG_CONFIG_PATH="$pc_dir${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
            break
        fi
    done
fi
for module in sdl3 sdl3-image sdl3-ttf sdl3-mixer; do
    pkg-config --exists "$module" 2>/dev/null \
        || die "SDL3 module '$module' not found (install the SDL3 development files)"
done

log "Configuring (SDL3 frontend, sound enabled)"
# This code base predates -fno-common (the GCC default since version 10);
# global variables are defined in headers, so -fcommon is required.
CFLAGS="${CFLAGS:--O2} -fcommon" ./configure --enable-sdl \
    >"$BUILD_DIR/configure.log" 2>&1 \
    || { tail -n 40 "$BUILD_DIR/configure.log" >&2; die "configure failed"; }

log "Building with $JOBS job(s)"
make -j"$JOBS" >"$BUILD_DIR/build.log" 2>&1 \
    || { tail -n 40 "$BUILD_DIR/build.log" >&2; die "build failed"; }

[ -x ./xblast ] || die "build did not produce ./xblast"

# ---------------------------------------------------------------------------
# Stage the AppDir
# ---------------------------------------------------------------------------
log "Staging AppDir"
mkdir -p \
    "$APPDIR/usr/bin" \
    "$APPDIR/usr/share/applications" \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps" \
    "$APPDIR/usr/share/doc/${APP_ID}" \
    "$APPDIR/usr/share/${APP_ID}"

install -m 755 ./xblast "$APPDIR/usr/bin/xblast"
if [ "$STRIP" -eq 1 ] && command -v strip >/dev/null 2>&1; then
    strip --strip-unneeded "$APPDIR"/usr/bin/* 2>/dev/null || true
fi

# Bundle the SDL3 runtime libraries together with everything they link
# against.  This is what makes the AppImage independent of the SDL3 and
# codec packages installed on the target machine: SDL3_mixer pulls in
# libfluidsynth/libxmp/libgme/libwavpack/... and SDL3_ttf needs FreeType.
#
# The C library, the dynamic loader and the GPU/driver stack (OpenGL, drm,
# gbm) are left to the host, since they must match the running system.
log "Bundling SDL3 libraries and their dependencies"
mkdir -p "$APPDIR/usr/lib"

is_host_lib() {
    printf '%s\n' "$1" | grep -Eq \
        '^(ld-linux[^/]*|libc\.so|libm\.so|libmvec\.so|libdl\.so|libpthread\.so|librt\.so|libresolv\.so|libnsl\.so|libutil\.so|libanl\.so|libGL\.so|libGLESv1_CM\.so|libGLESv2\.so|libEGL\.so|libGLX\.so|libGLdispatch\.so|libOpenGL\.so|libglapi\.so|libdrm\.so|libgbm\.so|libvulkan\.so)(\..*)?$'
}

queue=()
while IFS= read -r lib; do
    [ -n "$lib" ] && queue+=("$lib")
done < <(ldd ./xblast | awk '/=>/ { print $3 }' | grep '/libSDL3' || true)
[ ${#queue[@]} -gt 0 ] || die "no SDL3 libraries found in the binary's dependencies"

declare -A seen=()
for ((i = 0; i < ${#queue[@]}; i++)); do
    lib=${queue[i]}
    [ -n "${seen[$lib]:-}" ] && continue
    seen[$lib]=1
    if is_host_lib "$(basename "$lib")"; then
        continue
    fi
    cp -L "$lib" "$APPDIR/usr/lib/" || die "failed to copy $lib"
    # follow the dependencies of the library we just bundled
    while IFS= read -r dep; do
        [ -n "$dep" ] && queue+=("$dep")
    done < <(ldd "$lib" 2>/dev/null | awk '/=>/ { print $3 }' | grep '^/' || true)
done

for pattern in 'libSDL3.so.*' 'libSDL3_image.so.*' 'libSDL3_ttf.so.*' 'libSDL3_mixer.so.*'; do
    compgen -G "$APPDIR/usr/lib/$pattern" >/dev/null \
        || die "missing bundled library matching $pattern"
done

# Bundle a TrueType font.  The SDL3 frontend asks for "Vera.ttf" and searches
# the working directory, so a Vera or DejaVu font is installed under that name.
font_src=""
for candidate in \
    /usr/share/fonts/truetype/Vera.ttf \
    /usr/share/fonts/TTF/Vera.ttf \
    /usr/share/fonts/truetype/ttf-bitstream-vera/Vera.ttf \
    /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf \
    /usr/share/fonts/TTF/DejaVuSans.ttf \
    /usr/share/fonts/dejavu/DejaVuSans.ttf; do
    if [ -f "$candidate" ]; then
        font_src=$candidate
        break
    fi
done
[ -n "$font_src" ] \
    || die "no Vera or DejaVu TrueType font found (install ttf-bitstream-vera or dejavu-fonts)"
install -m 644 "$font_src" "$APPDIR/usr/share/${APP_ID}/Vera.ttf"

# Game data: image/, level/, sounds/
for dir in image level sounds; do
    [ -d "$dir" ] && cp -a "$dir" "$APPDIR/usr/share/${APP_ID}/"
done

# Translations.  The game resolves LOCALEDIR relative to its data directory
# (GAME_DATADIR "./"), and AppRun changes into that directory before starting.
shopt -s nullglob
for gmo in po/*.gmo; do
    lang=$(basename "$gmo" .gmo)
    install -d "$APPDIR/usr/share/${APP_ID}/locale/${lang}/LC_MESSAGES"
    install -m 644 "$gmo" \
        "$APPDIR/usr/share/${APP_ID}/locale/${lang}/LC_MESSAGES/xblast.mo"
done
shopt -u nullglob

install -m 644 COPYING "$APPDIR/usr/share/doc/${APP_ID}/COPYING" 2>/dev/null || true
install -m 644 README  "$APPDIR/usr/share/doc/${APP_ID}/README"  2>/dev/null || true

# Icon: the 32x32 bomb icon from the Windows resources, scaled up as pixel art.
"$IM" "$SRC_DIR/xblast.ico" -filter point -resize 256x256 \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps/${APP_ID}.png"

# Desktop entry
cat >"$APPDIR/usr/share/applications/${DESKTOP_NAME}" <<EOF
[Desktop Entry]
Type=Application
Name=${APP_NAME}
GenericName=Arcade Game
Comment=Multi-player "Blast the Others" game
Exec=xblast
Icon=${APP_ID}
Terminal=false
Categories=Game;ArcadeGame;
Keywords=xblast;bomberman;blast;
StartupNotify=false
EOF

# Root-level symlinks required by the AppImage specification
ln -sf "usr/share/applications/${DESKTOP_NAME}" "$APPDIR/${DESKTOP_NAME}"
ln -sf "usr/share/icons/hicolor/256x256/apps/${APP_ID}.png" "$APPDIR/${APP_ID}.png"
ln -sf "${APP_ID}.png" "$APPDIR/.DirIcon"

# AppRun: make the bundled data, libraries and font discoverable.  The game
# looks for its data in $XBLASTDIR and in the current directory, and finds the
# bundled "Vera.ttf" in the data directory.
cat >"$APPDIR/AppRun" <<'EOF'
#!/bin/sh
set -e
APPDIR="${APPDIR:-$(cd -- "$(dirname -- "$0")" && pwd)}"
export XBLASTDIR="$APPDIR/usr/share/xblast-tnt"
export LD_LIBRARY_PATH="$APPDIR/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$XBLASTDIR"
exec "$APPDIR/usr/bin/xblast" "$@"
EOF
chmod 755 "$APPDIR/AppRun"

# ---------------------------------------------------------------------------
# Pack the AppImage
# ---------------------------------------------------------------------------
CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/xblast-tnt-appimage"

fetch() { # fetch URL DEST
    if command -v curl >/dev/null 2>&1; then
        curl -fL --retry 3 -o "$2" "$1"
    elif command -v wget >/dev/null 2>&1; then
        wget -O "$2" "$1"
    else
        die "cannot download $1: neither curl nor wget is available"
    fi
}

is_appimage() {
    [ -f "$1" ] && [ "$(dd if="$1" bs=1 skip=8 count=2 2>/dev/null)" = "AI" ]
}

if command -v appimagetool >/dev/null 2>&1; then
    APPIMAGETOOL=$(command -v appimagetool)
else
    APPIMAGETOOL="$CACHE_DIR/appimagetool-${ARCH}.AppImage"
    if [ ! -x "$APPIMAGETOOL" ]; then
        log "Downloading appimagetool"
        mkdir -p -- "$CACHE_DIR"
        fetch "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-${ARCH}.AppImage" \
            "$APPIMAGETOOL"
        chmod +x "$APPIMAGETOOL"
    fi
fi

log "Packing AppImage"
export ARCH
rm -f -- "$OUTPUT"
LOG="$BUILD_DIR/appimagetool.log"

# Run appimagetool from its extracted form to avoid a FUSE self-mount, and
# hand it a cached runtime so it does not have to download one mid-build.
PACK=("$APPIMAGETOOL")
if is_appimage "$APPIMAGETOOL"; then
    PACK+=(--appimage-extract-and-run)
    RUNTIME_FILE="$CACHE_DIR/runtime-${ARCH}"
    if [ ! -f "$RUNTIME_FILE" ]; then
        log "Downloading AppImage runtime"
        mkdir -p -- "$CACHE_DIR"
        if ! fetch "https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-${ARCH}" \
                "$RUNTIME_FILE"; then
            rm -f -- "$RUNTIME_FILE"
        fi
    fi
    [ -f "$RUNTIME_FILE" ] && PACK+=(--runtime-file "$RUNTIME_FILE")
fi
PACK+=("$APPDIR" "$OUTPUT")

if ! "${PACK[@]}" 2>&1 | tee "$LOG"; then
    die "appimagetool failed (see $LOG)"
fi

[ -f "$OUTPUT" ] || die "appimagetool did not produce $OUTPUT"
chmod 755 "$OUTPUT"

log "Done: $OUTPUT"
log "Size: $(du -h "$OUTPUT" | cut -f1)"
