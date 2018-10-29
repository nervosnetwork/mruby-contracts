CC := riscv64-unknown-elf-gcc
MUSL_LIB := build/musl/lib/libc.a
MRUBY_LIB := mruby/build/riscv-gcc/lib/libmruby.a
FLATCC := flatcc/bin/flatcc

$(MUSL_LIB):
	cd riscv-musl && \
		./configure --prefix=../build/musl --target=riscv64-unknown-elf --disable-shared && \
		make && make install

$(MRUBY_LIB):
	cd mruby && \
		MUSL=../build/musl MRUBY_CONFIG=../build_config.rb make

$(FLATCC):
	cd flatcc && scripts/build.sh
