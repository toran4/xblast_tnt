# Building XBlast TNT

This document describes how to build XBlast TNT from source.

## Linux

Requirements: `gcc`/`clang`, `make`, `autoconf`/`automake`, and for the SDL3
frontend the SDL3 development files (`sdl3`, `sdl3-image`, `sdl3-ttf`,
`sdl3-mixer`).

```sh
./autogen.sh
./configure --enable-sdl          # SDL3 frontend (recommended)
make -j"$(nproc)"
./xblast
```

Other useful configure options:

```sh
./configure --enable-sound        # X11 frontend with sound (xbsndsrv)
./configure --enable-sdl --enable-mini --enable-SMPF
./configure --enable-admin --prefix=/usr   # system-wide install
make install
```

## Build matrix (CI)

The project is continuously built in GitHub Actions (`.github/workflows/build.yml`)
for:

- Linux, SDL3 frontend (`--enable-sdl`)
- Linux, X11 frontend with sound (`--enable-sound`)
- Windows (MinGW) via `packaging/windows/Makefile.windows`

A separate `quality.yml` workflow builds with AddressSanitizer/UndefinedBehaviorSanitizer
and runs clang-tidy on changed files.

## Windows (MinGW)

```sh
make -f packaging/windows/Makefile.windows
```

## Cygwin

```sh
make -f packaging/windows/Makefile.windows
```

## Notes

- The C sources and headers live in `src/`; object files and dependency data
  are generated in `src/` as well, so the repository root stays clean.
- Autotools helper scripts live in `build-aux/`, Windows packaging files in
  `packaging/windows/`.
- Run `./autogen.sh` after changing `configure.in` or `Makefile.am`.
- `AUTOMAKE_OPTIONS = foreign` is set so automake does not require/ship
  standard GNU files such as `COPYING` or `NEWS`; the project uses `LICENSE`.
- For a headless central server today, run the normal binary under a virtual
  display: `xvfb-run -a ./xblast -central`.
```