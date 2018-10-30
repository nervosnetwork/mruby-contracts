MRuby::Toolchain.new(:riscv) do |conf, _params|
  toolchain :gcc

  [conf.cc, conf.objc, conf.asm].each do |cc|
    cc.command = ENV['CC'] || 'riscv64-unknown-elf-gcc'
  end
  conf.archiver.command = ENV['AR'] || 'riscv64-unknown-elf-ar'
  conf.cxx.command = ENV['CXX'] || 'riscv64-unknown-elf-g++'
  conf.linker.command = ENV['LD'] || 'riscv64-unknown-elf-gcc'
end

MRuby::Toolchain.new(:riscv_newlib) do |conf, _params|
  toolchain :riscv

  conf.cc.flags = [ENV['CFLAGS'] || %w(-specs ../newlib-gcc.specs -O2)]
  conf.linker.flags = [ENV['LDFLAGS'] || %w(-specs ../newlib-gcc.specs)]
end

MRuby::Build.new do |conf|
  toolchain :gcc

  conf.cc.flags << "-I ../flatcc/include"

  conf.enable_bintest
  conf.enable_test

  # Use standard Array#pack, String#unpack methods
  conf.gem :core => "mruby-pack"

  # Use standard Kernel#sprintf method
  conf.gem :core => "mruby-sprintf"

  # Use standard print/puts/p
  conf.gem :core => "mruby-print"

  # Use standard Math module
  conf.gem :core => "mruby-math"

  # Use standard Time class
  conf.gem :core => "mruby-time"

  # Use standard Struct class
  conf.gem :core => "mruby-struct"

  # Use Comparable module extension
  conf.gem :core => "mruby-compar-ext"

  # Use Enumerable module extension
  conf.gem :core => "mruby-enum-ext"

  # Use String class extension
  conf.gem :core => "mruby-string-ext"

  # Use Numeric class extension
  conf.gem :core => "mruby-numeric-ext"

  # Use Array class extension
  conf.gem :core => "mruby-array-ext"

  # Use Hash class extension
  conf.gem :core => "mruby-hash-ext"

  # Use Range class extension
  conf.gem :core => "mruby-range-ext"

  # Use Proc class extension
  conf.gem :core => "mruby-proc-ext"

  # Use Symbol class extension
  conf.gem :core => "mruby-symbol-ext"

  # Use Random class
  conf.gem :core => "mruby-random"

  # Use Object class extension
  conf.gem :core => "mruby-object-ext"

  # Use ObjectSpace class
  conf.gem :core => "mruby-objectspace"

  # Use Fiber class
  conf.gem :core => "mruby-fiber"

  # Use Enumerator class (require mruby-fiber)
  conf.gem :core => "mruby-enumerator"

  # Use Enumerator::Lazy class (require mruby-enumerator)
  conf.gem :core => "mruby-enum-lazy"

  # Use toplevel object (main) methods extension
  conf.gem :core => "mruby-toplevel-ext"

  # Generate mirb command
  conf.gem :core => "mruby-bin-mirb"

  # Generate mruby command
  conf.gem :core => "mruby-bin-mruby"

  # Generate mruby-strip command
  conf.gem :core => "mruby-bin-strip"

  # Use Kernel module extension
  conf.gem :core => "mruby-kernel-ext"

  # Use class/module extension
  conf.gem :core => "mruby-class-ext"

  # Use mruby-compiler to build other mrbgems
  conf.gem :core => "mruby-compiler"

  conf.gem "mruby-ckb"
end

MRuby::Build.new('riscv-gcc-spike') do |conf|
  toolchain :riscv_newlib

  conf.cc.flags << "-I ../flatcc/include"

  conf.enable_bintest
  conf.enable_test

  # Use standard Array#pack, String#unpack methods
  conf.gem :core => "mruby-pack"

  # Use standard Kernel#sprintf method
  conf.gem :core => "mruby-sprintf"

  # Use standard print/puts/p
  conf.gem :core => "mruby-print"

  # Use standard Math module
  conf.gem :core => "mruby-math"

  # Use standard Time class
  conf.gem :core => "mruby-time"

  # Use standard Struct class
  conf.gem :core => "mruby-struct"

  # Use Comparable module extension
  conf.gem :core => "mruby-compar-ext"

  # Use Enumerable module extension
  conf.gem :core => "mruby-enum-ext"

  # Use String class extension
  conf.gem :core => "mruby-string-ext"

  # Use Numeric class extension
  conf.gem :core => "mruby-numeric-ext"

  # Use Array class extension
  conf.gem :core => "mruby-array-ext"

  # Use Hash class extension
  conf.gem :core => "mruby-hash-ext"

  # Use Range class extension
  conf.gem :core => "mruby-range-ext"

  # Use Proc class extension
  conf.gem :core => "mruby-proc-ext"

  # Use Symbol class extension
  conf.gem :core => "mruby-symbol-ext"

  # Use Random class
  conf.gem :core => "mruby-random"

  # Use Object class extension
  conf.gem :core => "mruby-object-ext"

  # Use ObjectSpace class
  conf.gem :core => "mruby-objectspace"

  # Use Fiber class
  conf.gem :core => "mruby-fiber"

  # Use Enumerator class (require mruby-fiber)
  conf.gem :core => "mruby-enumerator"

  # Use Enumerator::Lazy class (require mruby-enumerator)
  conf.gem :core => "mruby-enum-lazy"

  # Use toplevel object (main) methods extension
  conf.gem :core => "mruby-toplevel-ext"

  # Generate mirb command
  conf.gem :core => "mruby-bin-mirb"

  # Generate mruby command
  conf.gem :core => "mruby-bin-mruby"

  # Generate mruby-strip command
  conf.gem :core => "mruby-bin-strip"

  # Use Kernel module extension
  conf.gem :core => "mruby-kernel-ext"

  # Use class/module extension
  conf.gem :core => "mruby-class-ext"

  # Use mruby-compiler to build other mrbgems
  conf.gem :core => "mruby-compiler"

  conf.gem "mruby-ckb"
end

MRuby::Build.new('riscv-gcc') do |conf|
  toolchain :riscv_newlib

  conf.cc.flags << "-I ../flatcc/include -DMRB_WITHOUT_FLOAT -DMRB_DISABLE_STDIO -DCKB_HAS_SYSCALLS"

  # Use standard Array#pack, String#unpack methods
  conf.gem :core => "mruby-pack"

  # Use standard Struct class
  conf.gem :core => "mruby-struct"

  # Use Comparable module extension
  conf.gem :core => "mruby-compar-ext"

  # Use Enumerable module extension
  conf.gem :core => "mruby-enum-ext"

  # Use String class extension
  conf.gem :core => "mruby-string-ext"

  # Use Array class extension
  conf.gem :core => "mruby-array-ext"

  # Use Hash class extension
  conf.gem :core => "mruby-hash-ext"

  # Use Proc class extension
  conf.gem :core => "mruby-proc-ext"

  # Use Symbol class extension
  conf.gem :core => "mruby-symbol-ext"

  # Use Object class extension
  conf.gem :core => "mruby-object-ext"

  # Use ObjectSpace class
  conf.gem :core => "mruby-objectspace"

  # Use Fiber class
  conf.gem :core => "mruby-fiber"

  # Use Enumerator class (require mruby-fiber)
  conf.gem :core => "mruby-enumerator"

  # Use Enumerator::Lazy class (require mruby-enumerator)
  conf.gem :core => "mruby-enum-lazy"

  # Use toplevel object (main) methods extension
  conf.gem :core => "mruby-toplevel-ext"

  # Use Kernel module extension
  conf.gem :core => "mruby-kernel-ext"

  # Use class/module extension
  conf.gem :core => "mruby-class-ext"

  # Use mruby-compiler to build other mrbgems
  conf.gem :core => "mruby-compiler"

  conf.gem "mruby-ckb"
end
