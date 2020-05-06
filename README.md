# FS-2

This repo contains all the software for the fs-2 racecar running on mbed os 5 on the LPC1768 microcontroller.

## Projects

- `bms`: LTC6811-based bms code for use with the fs-2 accumulator

## Workflow

The workflow should be used as follows.

### Git

Features/fixes can only be added with a pull request. Branches should be named as follows.

- Fix: `fix/<short description>`
- Feature: `feature/<short description>`

### Required tools
- [mbed cli](https://github.com/ARMmbed/mbed-cli)
- [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
    - Ubuntu/Debian: `gcc-arm-none-eabi`
    - Brew: `gcc-arm-embedded`
    - Nix: `nixpkgs.gcc-arm-embedded`
- A unix shell

### Installing dependencies

A project's dependencies can be installed by running the following command in the project folder.

```
mbed update
```

This will produce warnings about not using source control management. These can be safely ignored.

### Compiling

A project can be compiled using

```
mbed compile
```

or if you wish to flash the board after compiling

```
mbed compile -f
```

### Connecting to serial terminal

All of our boards use a rate of 115200 baud.

```
mbed sterm -b 115200
```

### Adding libraries

Libraries can be added to a project using

```
mbed add <url>
```

then the corresponding directory should be added to the `.gitignore` in the project's directory.

### All other issues

For any other issues see the [mbed cli documentation](https://os.mbed.com/docs/mbed-os/v5.15/tools/working-with-mbed-cli.html).
