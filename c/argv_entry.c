#include <stdint.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/irep.h>

#ifdef SECP256K1_CUSTOM_FUNCS
#include "ckb_syscalls.h"
void custom_abort()
{
  syscall(SYS_exit, 10, 0, 0, 0, 0, 0);
}

int custom_print_err(const char * arg, ...)
{
  (void) arg;
  return 0;
}
#endif

int main(int argc, char* argv[])
{
  if (argc < 2) {
    return -2;
  }

  mrb_state *mrb = mrb_open();

  mrb_value margv = mrb_ary_new_capa(mrb, argc);
  for (int i = 2; i < argc; i++) {
    char* utf8 = mrb_utf8_from_locale(argv[i], -1);
    if (utf8) {
      mrb_ary_push(mrb, margv, mrb_str_new_cstr(mrb, utf8));
      mrb_utf8_free(utf8);
    }
  }
  mrb_define_global_const(mrb, "ARGV", margv);

  mrb_value v = mrb_load_irep(mrb, argv[1]);

  if (mrb->exc) {
#ifndef MRB_DISABLE_STDIO
    if (mrb_undef_p(v)) {
      mrb_p(mrb, mrb_obj_value(mrb->exc));
    }
    else {
      mrb_print_error(mrb);
    }
#endif
    return -1;
  }

  return 0;
}
