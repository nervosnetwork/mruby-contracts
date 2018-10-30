#include <stdint.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/irep.h>

int main(int argc, char* argv[])
{
  if (argc < 1) {
    return -2;
  }

  mrb_state *mrb = mrb_open();

  mrb_value margv = mrb_ary_new_capa(mrb, argc);
  for (int i = 1; i < argc; i++) {
    char* utf8 = mrb_utf8_from_locale(argv[i], -1);
    if (utf8) {
      mrb_ary_push(mrb, margv, mrb_str_new_cstr(mrb, utf8));
      mrb_utf8_free(utf8);
    }
  }
  mrb_define_global_const(mrb, "ARGV", margv);

  mrb_value v = mrb_load_irep(mrb, argv[0]);

  if (mrb->exc) {
    return -1;
  }

  return 0;
}
