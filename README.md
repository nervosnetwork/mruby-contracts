# CKB mruby contract

---

## About CKB mruby contract

CKB mruby contract is a port of [mruby](https://github.com/mruby/mruby) on CKB VM. It enables writing CKB contracts using plain Ruby language. In addition to off-the-shell mruby, it has the following additional mrbgems:

* `mruby-ckb`: provides access to CKB environment, such as loading transaction, script hash, cell data as well as debugging support.
* `mruby-secp256k1`: provides secp256k1 binding in Ruby environment
* `mruby-sha3`: provides a basic SHA3 binding in Ruby environment

## How to build

In order to build this project, you should have RISC-V compilers available. One quicker way would be leveraging [riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain):

```bash
$ mkdir riscv
$ export RISCV=$(pwd)/riscv
$ git clone https://github.com/riscv/riscv-gnu-toolchain
$ cd riscv-gnu-toolchain
$ git submodule update --init --recursive --progress
# ckb-vm doesn't provide floating point support
$ ./configure --prefix=$RISCV --with-arch=rv64imac
$ make -j$(nproc)
```

When you are done building riscv-gnu-toolchain, make sure to add the path to final binaries(`$RISCV/bin`) to `PATH`. One way to test this is `which riscv64-unknown-elf-gcc`.

Now you should be ready to build this project:

```bash
$ git clone https://github.com/nervosnetwork/mruby-contracts
$ cd mruby-contracts
$ git submodule update --init --progress
$ make
```

If everything works okay, you should have the contract to use at `build/argv_source_entry`.
