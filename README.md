# XBlast TNT

[![Build](https://github.com/toran4/xblast_tnt/actions/workflows/build.yml/badge.svg)](https://github.com/toran4/xblast_tnt/actions/workflows/build.yml)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](LICENSE)

XBlast TNT is a free, open-source, multi-player **Bomberman-style arcade game**.
Up to six players (sixteen with the optional SMPF build) battle it out on a
shrinking arena, collecting extras and blasting each other with bombs.

XBlast TNT, created by Oliver Vogel as a rewrite of his original XBlast game, was 
later modified by the community. This project is a fork and modernisation of the 
community mod, which introduced a central server for score tracking and game 
discovery, along with additional features and gameplay enhancements.

This repository keeps the game building on modern systems: an **SDL3** frontend
(with bundled sound), an **X11** frontend, **Windows (MinGW)** builds, and a
self-contained **Linux AppImage**.

## Features

- Local and network multiplayer (LAN and internet/central games)
- 1200+ bundled levels
- Customisable key bindings, skins and levels
- Translated UI (`.po`/`.mo`)
- Optional 16-player mode (`--enable-SMPF`), mini window (`--enable-mini`)

## Quick start

### Linux (AppImage)

A pre-built AppImage is available in [`dist/`](dist/) (or build your own with
[`build-appimage.sh`](build-appimage.sh)):

```sh
chmod +x xblast-tnt-*.AppImage
./xblast-tnt-*.AppImage
```

### Build from source (Linux)

Requirements: `gcc`/`clang`, `make`, `autoconf`/`automake`, and for the SDL3
frontend the SDL3 development files (`sdl3`, `sdl3-image`, `sdl3-ttf`,
`sdl3-mixer`).

```sh
./autogen.sh
./configure --enable-sdl          # SDL3 frontend (recommended)
make -j"$(nproc)"
./xblast
```

Other useful configure options (see [`INSTALL`](INSTALL)):

```sh
./configure --enable-sound        # X11 frontend with sound (xbsndsrv)
./configure --enable-sdl --enable-mini --enable-SMPF
./configure --enable-admin --prefix=/usr   # system-wide install
make install
```

### Windows (MinGW)

```sh
make -f Makefile.windows
```

### Cygwin

```sh
make -f Makefile.windows
```


## Project layout

- `*.c` / `*.h` — game source
- `sdl_*` / `x11_*` / `w32_*` — frontend backends
- `image/`, `level/`, `sounds/` — game data
- `po/` — translations
- `build-appimage.sh` — Linux AppImage packaging
- `.github/workflows/build.yml` — CI (Linux SDL3/X11, Windows MinGW)

## Contributing

Bug reports and pull requests are welcome via
[GitHub Issues](https://github.com/toran4/xblast_tnt/issues). Historical
contact points (mailing lists, IRC) may no longer be active.

## License

XBlast TNT is free software released under the **GNU General Public License v2
(or later)**. See [`LICENSE`](LICENSE).

## Credits

- **Oliver Vogel** — original author
- **Koen, Skywalker** — mod, central server, EPFL features
- Plus many contributors listed in [`xblast.man`](xblast.man)

See [`AUTHORS`](AUTHORS) and [`ChangeLog`](ChangeLog) for more information
from historical development of the game.