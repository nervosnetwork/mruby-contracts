#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/string.h>

#include "sha3.h"

static char const SHA3_STATE_KEY[] = "$mrb_i_sha3_ctx";

static const struct mrb_data_type sha3_state_type = {
  SHA3_STATE_KEY, mrb_free,
};

static mrb_value
mrb_sha3_init(mrb_state *mrb, mrb_value self)
{
  mrb_int mdlen = 32;
  sha3_ctx_t *t;

  mrb_get_args(mrb, "|i", &mdlen);

  t = (sha3_ctx_t *) mrb_malloc(mrb, sizeof(sha3_ctx_t));
  mrb_data_init(self, t, &sha3_state_type);

  return self;
}

static mrb_value
mrb_sha3_update(mrb_state *mrb, mrb_value self)
{
  char *message_buf;
  size_t message_len;
  sha3_ctx_t *t;

  mrb_get_args(mrb, "s", &message_buf, &message_len);

  t = DATA_GET_PTR(mrb, self, &sha3_state_type, sha3_ctx_t);
  sha3_update(t, message_buf, message_len);

  return self;
}

static mrb_value
mrb_sha3_final(mrb_state *mrb, mrb_value self)
{
  mrb_value s;
  sha3_ctx_t *t = DATA_GET_PTR(mrb, self, &sha3_state_type, sha3_ctx_t);

  s = mrb_str_new_capa(mrb, t->mdlen);
  sha3_final(RSTRING_PTR(s), t);

  return s;
}

void
mrb_mruby_sha3_gem_init(mrb_state* mrb)
{
  struct RClass *sha3;
  sha3 = mrb_define_class(mrb, "Sha3", mrb->object_class);
  MRB_SET_INSTANCE_TT(sha3, MRB_TT_DATA);

  mrb_define_method(mrb, sha3, "initialize", mrb_sha3_init, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, sha3, "update", mrb_sha3_update, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, sha3, "final", mrb_sha3_final, MRB_ARGS_NONE());
}

void
mrb_mruby_sha3_gem_final(mrb_state* mrb)
{
}
