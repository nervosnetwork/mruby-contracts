#include <stdint.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/compile.h>
#include <mruby/irep.h>

#ifdef SECP256K1_CUSTOM_FUNCS
#include <machine/syscall.h>
void custom_abort()
{
  syscall_errno(93, 10, 0, 0, 0, 0, 0);
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

  mrb_value v;
  if (argv[1][0] == 'E' && argv[1][1] == 'T' &&
      argv[1][2] == 'I' && argv[1][3] == 'R') {
    v = mrb_load_irep(mrb, argv[1]);
  } else {
    v = mrb_load_string(mrb, argv[1]);
  }

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
