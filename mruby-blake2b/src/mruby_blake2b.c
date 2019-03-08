#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/string.h>

#include "blake2b.h"

static char const BLAKE2B_STATE_KEY[] = "$mrb_i_blake2b_state";

static const struct mrb_data_type blake2b_state_type = {
  BLAKE2B_STATE_KEY, mrb_free,
};

static mrb_value
mrb_blake2b_init(mrb_state *mrb, mrb_value self)
{
  // mrb_int outlen = 32;
  size_t outlen = 32;
  blake2b_state *s;

  // mrb_get_args(mrb, "|i", &outlen);
  mrb_get_args(mrb, "s", &outlen);

  s = (blake2b_state *) mrb_malloc(mrb, sizeof(blake2b_state));
  blake2b_init(s, outlen);
  mrb_data_init(self, s, &blake2b_state_type);

  return self;
}

static mrb_value
mrb_blake2b_update(mrb_state *mrb, mrb_value self)
{
  char *message_buf;
  size_t message_len;
  blake2b_state *s;

  mrb_get_args(mrb, "s", &message_buf, &message_len);
  
  s = DATA_GET_PTR(mrb, self, &blake2b_state_type, blake2b_state);
  blake2b_update(s, message_buf, message_len);

  return self;
}

static mrb_value
mrb_blake2b_final(mrb_state *mrb, mrb_value self)
{
  mrb_value s;
  blake2b_state *t = DATA_GET_PTR(mrb, self, &blake2b_state_type, blake2b_state);

  s = mrb_str_new_capa(mrb, t->outlen);
  blake2b_final(t, RSTRING_PTR(s), t->outlen);
  RSTR_SET_LEN(mrb_str_ptr(s), t->outlen);

  return s;
}

static mrb_value
mrb_blake2b_outlen(mrb_state *mrb, mrb_value self)
{
  blake2b_state *t = DATA_GET_PTR(mrb, self, &blake2b_state_type, blake2b_state);

  return mrb_fixnum_value(t->outlen);
}

void
mrb_mruby_blake2b_gem_init(mrb_state *mrb)
{
  struct RClass *blake2b;
  blake2b = mrb_define_class(mrb, "Blake2b", mrb->object_class);
  MRB_SET_INSTANCE_TT(blake2b, MRB_TT_DATA);

  mrb_define_method(mrb, blake2b, "initialize", mrb_blake2b_init, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, blake2b, "update", mrb_blake2b_update, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, blake2b, "final", mrb_blake2b_final, MRB_ARGS_NONE());
  mrb_define_method(mrb, blake2b, "outlen", mrb_blake2b_outlen, MRB_ARGS_NONE());
}

void
mrb_mruby_blake2b_gem_final(mrb_state *mrb)
{
}



