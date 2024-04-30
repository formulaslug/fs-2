# FS-2

This repo contains all the software for the fs-2 racecar running on mbed os 6 on the Nucleo L432KC microcontroller.

## Projects

- `bms`: LTC6811-based bms code for use with the fs-2 accumulator

## Getting Started

There are two ways to get started with fs-2 development: Docker and native. Docker is the recommended method because it is the easiest to set up and is the best documented.

### Method 1: Docker (Recommended)

Docker is the easiest method to get a fully functional toolchain. Docker containers allow for pre-packaged environments to be distributed with minimal setup.

First, install docker [here](https://docs.docker.com/get-docker).

Next, to download the docker container with the Mbed toolchain installed, run

```sh
docker pull ghcr.io/armmbed/mbed-os-env:mbed-os-6-latest
```

And then to start a container with the current directory mounted run

```sh
docker run -it --rm --mount=type=bind,source="$(pwd)",destination=/var/mbed -w /var/mbed ghcr.io/armmbed/mbed-os-env:mbed-os-6-latest
```

See [the Mbed Docker documentation](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/docker.html) if you have issues getting the container started.

### Method 2: Native

This last method requires the tools be installed manually. The full list of required tools is given below.

- [mbed-tools](https://github.com/ARMmbed/mbed-tools)
- [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)
    - Ubuntu/Debian: `gcc-arm-none-eabi`
    - Brew: `gcc-arm-embedded`
    - Nix: `nixpkgs.gcc-arm-embedded`
- [Make](https://www.gnu.org/software/make/)
- [CMake](https://cmake.org/)
- Your favorite unix shell

## Workflow

The workflow should be used as follows.

### Git

Features/fixes can only be added with a pull request. Branches should be named as follows.

- Fix: `fix/<short description>`
- Feature: `feature/<short description>`

### All other issues

For any other issues see the [mbed documentation](https://os.mbed.com/docs/mbed-os).
