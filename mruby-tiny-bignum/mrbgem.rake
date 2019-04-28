MRuby::Gem::Specification.new("mruby-tiny-bignum") do |spec|
  spec.license = "Unlicensed"
  spec.author  = "Nervos Core Dev <dev@nervos.org>"
  spec.summary = "mruby wrapper for tiny-bignum-c"

  tiny_bignum_path = "#{spec.dir}/../tiny-bignum-c"
  spec.cc.include_paths << tiny_bignum_path
  spec.objs += Dir.glob("#{tiny_bignum_path}/*.c").map { |f| f.relative_path_from(dir).pathmap("#{build_dir}/%X.o") }
end
