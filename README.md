# nbtq
[jq](https://jqlang.org/) for NBT

Simple but powerful command-line tool to query and convert [NBT](https://minecraft.wiki/w/NBT_format) Data

> [!WARNING]
> This is under construction! ⚠️🚧⚠️🚧⚠️🚧.
> Use https://github.com/C4K3/nbted for something that actually works

## Compiling

### Prerequisites
- c compiler (gcc)
- zlib development files
- linux
- electricity (optional)

##### Void Linux
```console
xbps-install zlib-devel
```

##### Arch Linux
```console
pacman -S zlib-devel
```

##### Debian
```console
apt install zlib1g-dev
```

### Instructions
```console
cc nob.c -o nob
./nob
```
