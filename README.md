# CKB mruby contract

---

## About CKB mruby contract

CKB mruby contract is a port of [mruby](https://github.com/mruby/mruby) on CKB VM. It enables writing CKB contracts using plain Ruby language. In addition to off-the-shell mruby, it has the following additional mrbgems:

* `mruby-ckb`: provides access to CKB environment, such as loading transaction, script hash, cell data as well as debugging support.
* `mruby-secp256k1`: provides secp256k1 binding in Ruby environment
* `mruby-blake2b`: provides a basic Blake2b binding in Ruby environment(hard-code personalization to "ckb-default-hash")

## How to build dependencies

In order to build this project, you should have RISC-V compilers available. There're several ways you can install them:

### riscv-gnu-toolchain

You can install [riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain) with following steps:

```bash
$ mkdir riscv
$ export RISCV=$(pwd)/riscv
$ git clone https://github.com/riscv/riscv-gnu-toolchain
$ cd riscv-gnu-toolchain
$ git submodule update --init --recursive --progress
# ckb-vm doesn't provide floating point support
$ ./configure --prefix=$RISCV --with-arch=rv64imac
# On macOS use `make -j$(getconf _NPROCESSORS_ONLN)` instead as there's no nproc command
$ make -j$(nproc)
```

This way will give you latest gcc versions(as of writing of this doc, riscv-gnu-toolchain contains the latest gcc 8.2), however, one quirk we found is that you can only build this on Linux, there's a [bug](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=86724) preventing building this on MacOS.

### riscv-tools

You can also use [riscv-tools](https://github.com/riscv/riscv-tools), which is more stable but not always up-to-date with latest gcc. To build riscv-tools, first clone the repo:

```bash
$ mkdir riscv
$ export RISCV=$(pwd)/riscv
$ git clone https://github.com/riscv/riscv-tools
$ cd riscv-tools
$ git submodule update --init --recursive --progress
```

Since we need to build gcc without floating point number support, you need a separate build config from the stock ones. You can create a file named `build-rv64imac.sh` in `riscv-tools` repo's root folder with the following content:

```
. build.common

echo "Starting RISC-V Toolchain build process"

check_version() {
    $1 --version | awk "NR==1 {if (\$NF>$2) {exit 0} exit 1}" || (
        echo $3 requires at least version $2 of $1. Aborting.
        exit 1
    )
}

check_version automake 1.14 "OpenOCD build"
check_version autoconf 2.64 "OpenOCD build"
build_project riscv-openocd --prefix=$RISCV --enable-remote-bitbang --enable-jtag_vpi --disable-werror

build_project riscv-fesvr --prefix=$RISCV
build_project riscv-isa-sim --prefix=$RISCV --with-fesvr=$RISCV --with-isa=rv64imac
build_project riscv-gnu-toolchain --prefix=$RISCV --with-arch=rv64imac
CC= CXX= build_project riscv-pk --prefix=$RISCV --host=riscv64-unknown-elf
build_project riscv-tests --prefix=$RISCV/riscv64-unknown-elf

echo -e "\\nRISC-V Toolchain installation completed!"
```

Now you can build `riscv-tools`:

```bash
$ chmod a+x build-rv64imac.sh
$ ./build-rv64imac.sh
```

### Docker

If you don't want to build gcc by yourself, and happen to have docker available, we have prepared a docker image containing pre-built riscv-gnu-toolchain for you. You can change to `mruby-contracts` folder and launch docker with the following command:

```bash
$ docker run --rm -it -v `pwd`:/code xxuejie/riscv-gnu-toolchain-rv64imac bash
```

Now you will be in a Linux environment with riscv-gnu-toolchain available to use.

## How to build

When you have risc-v compilers, make sure to add the path to final binaries(`$RISCV/bin`) to `PATH`. One way to test this is `which riscv64-unknown-elf-gcc`.

Now you should be ready to build this project:

```bash
$ git clone https://github.com/nervosnetwork/mruby-contracts
$ cd mruby-contracts
$ git submodule update --init --progress
$ make
```

If everything works okay, you should have the contract to use at `build/argv_source_entry`.
