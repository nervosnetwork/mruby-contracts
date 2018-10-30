#include <stdint.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/irep.h>

extern const uint8_t code[];

int main(int argc, char* argv[])
{
  mrb_state *mrb = mrb_open();

  if (mrb->allocf == NULL) {
    return 123;
  }

  /* int ai = mrb_gc_arena_save(mrb); */
  mrb_value margv = mrb_ary_new_capa(mrb, argc);
  for (int i = 0; i < argc; i++) {
    char* utf8 = mrb_utf8_from_locale(argv[i], -1);
    if (utf8) {
      mrb_ary_push(mrb, margv, mrb_str_new_cstr(mrb, utf8));
      mrb_utf8_free(utf8);
    }
  }
  mrb_define_global_const(mrb, "ARGV", margv);

  mrb_value v = mrb_load_irep(mrb, code);

  /* mrb_gc_arena_restore(mrb, ai); */

  if (mrb->exc) {
    return 87;
  }

  return 0;
}
