#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/string.h>

#include <string.h>

#include "bn.h"

static char const TINY_BIGNUM_STATE_KEY[] = "$mrb_i_tiny_bignum";

static const struct mrb_data_type tiny_bignum_state_type = {
  TINY_BIGNUM_STATE_KEY, mrb_free,
};

static mrb_value new_tiny_bignum_value(mrb_state *mrb, struct bn *b)
{
  struct bn *p = (struct bn *) mrb_malloc(mrb, sizeof(struct bn));
  bignum_assign(p, b);

  return mrb_obj_value(mrb_data_object_alloc(
      mrb, mrb_class_get(mrb, "TinyBignum"), p, &tiny_bignum_state_type));
}

static DTYPE_TMP
get_value(mrb_state *mrb, mrb_int base, char ch)
{
  if (ch >= '0' && ch <= '9') {
    return (DTYPE_TMP) (ch - '0');
  }
  if (base == 16) {
    if (ch >= 'a' && ch <= 'f') {
      return (DTYPE_TMP) (ch - 'a' + 10);
    }
    if (ch >= 'A' && ch <= 'F') {
      return (DTYPE_TMP) (ch - 'A' + 10);
    }
  }
  mrb_raisef(mrb, E_ARGUMENT_ERROR, "invalid character: %S", mrb_fixnum_value(ch));
  return -1;
}

static mrb_value
fixnum_to_big(mrb_state *mrb, mrb_value self)
{
  struct bn tmp;

  bignum_from_int(&tmp, mrb_fixnum(self));
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
string_to_big(mrb_state *mrb, mrb_value self)
{
  mrb_int base = 10;
  struct bn n, ntmp, ntmp2;
  int i;
  DTYPE_TMP current, times, val, tmp;

  mrb_get_args(mrb, "|i", &base);
  if (base != 10 && base != 16) {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "invalid base %S", mrb_fixnum_value(base));
  }

  i = 0;
  if (RSTRING_PTR(self)[0] == '0' && RSTRING_PTR(self)[1] == 'x') {
    base = 16;
    i = 2;
  }

  bignum_init(&n);
  current = 0;
  times = 1;
  for (; i < RSTRING_LEN(self); i++) {
    val = get_value(mrb, base, RSTRING_PTR(self)[i]);
    tmp = current * base + val;
    if (tmp > MAX_VAL) {
      bignum_from_int(&ntmp, times);
      bignum_mul(&n, &ntmp, &ntmp2);
      bignum_from_int(&ntmp, current);
      bignum_add(&ntmp2, &ntmp, &n);

      current = val;
      times = base;
    } else {
      current = tmp;
      times *= base;
    }
  }
  if (times > 1) {
      bignum_from_int(&ntmp, times);
      bignum_mul(&n, &ntmp, &ntmp2);
      bignum_from_int(&ntmp, current);
      bignum_add(&ntmp2, &ntmp, &n);
  }

  return new_tiny_bignum_value(mrb, &n);
}

static mrb_value
mrb_bignum_to_s(mrb_state *mrb, mrb_value self)
{
  mrb_value s;
  struct bn *b = DATA_GET_PTR(mrb, self, &tiny_bignum_state_type, struct bn);
  size_t size;

  /* TODO: deal with base 10 printing later */
  s = mrb_str_new_capa(mrb, 2 + BN_ARRAY_SIZE * WORD_SIZE * 2 + 1);
  RSTRING_PTR(s)[0] = '0';
  RSTRING_PTR(s)[1] = 'x';
  bignum_to_string(b, &RSTRING_PTR(s)[2], BN_ARRAY_SIZE * WORD_SIZE * 2);
  size = strlen(RSTRING_PTR(s));
  if (size == 2) {
    RSTRING_PTR(s)[size] = '0';
    size++;
  }
  RSTR_SET_LEN(mrb_str_ptr(s), size);

  return s;
}

static mrb_value
mrb_bignum_add(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_add(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_sub(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_sub(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_mul(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_mul(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_div(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_div(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_mod(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_mod(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_and(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_and(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_or(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_or(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_xor(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_xor(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_lshift(mrb_state *mrb, mrb_value self)
{
  mrb_int bits;
  struct bn tmp;

  mrb_get_args(mrb, "i", &bits);

  if (bits < 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Negative shifts are not allowed!");
  }

  bignum_lshift(DATA_PTR(self), &tmp, bits);

  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_rshift(mrb_state *mrb, mrb_value self)
{
  mrb_int bits;
  struct bn tmp;

  mrb_get_args(mrb, "i", &bits);

  if (bits < 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Negative shifts are not allowed!");
  }

  bignum_rshift(DATA_PTR(self), &tmp, bits);

  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_cmp(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  int v;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  v = bignum_cmp(DATA_PTR(self), DATA_PTR(other));
  return mrb_fixnum_value(v);
}

static mrb_value
mrb_bignum_pow(mrb_state *mrb, mrb_value self)
{
  mrb_value other;
  struct bn tmp;

  mrb_get_args(mrb, "o", &other);

  if (mrb_type(other) != MRB_TT_DATA || DATA_TYPE(other) != &tiny_bignum_state_type) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Operand is not TinyBignum type!");
  }

  bignum_pow(DATA_PTR(self), DATA_PTR(other), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_isqrt(mrb_state *mrb, mrb_value self)
{
  struct bn tmp;

  bignum_isqrt(DATA_PTR(self), &tmp);
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_clone(mrb_state *mrb, mrb_value self)
{
  struct bn tmp;

  bignum_assign(&tmp, DATA_PTR(self));
  return new_tiny_bignum_value(mrb, &tmp);
}

static mrb_value
mrb_bignum_to_big(mrb_state *mrb, mrb_value self)
{
  (void) mrb;

  return self;
}


void mrb_mruby_tiny_bignum_gem_init(mrb_state *mrb)
{
  struct RClass *fixnum  = mrb->fixnum_class;
  struct RClass *string = mrb->string_class;
  struct RClass *tiny_bignum = mrb_define_class(mrb, "TinyBignum", mrb->object_class);
  MRB_SET_INSTANCE_TT(tiny_bignum, MRB_TT_DATA);
  mrb_include_module(mrb, tiny_bignum, mrb_module_get(mrb, "Comparable"));

  mrb_define_method(mrb, tiny_bignum, "to_s", mrb_bignum_to_s, MRB_ARGS_NONE());
  mrb_define_method(mrb, tiny_bignum, "+", mrb_bignum_add, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "-", mrb_bignum_sub, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "*", mrb_bignum_mul, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "/", mrb_bignum_div, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "%", mrb_bignum_mod, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "&", mrb_bignum_and, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "|", mrb_bignum_or, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "^", mrb_bignum_xor, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "<<", mrb_bignum_lshift, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, ">>", mrb_bignum_rshift, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "<=>", mrb_bignum_cmp, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "**", mrb_bignum_pow, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, tiny_bignum, "isqrt", mrb_bignum_isqrt, MRB_ARGS_NONE());
  mrb_define_method(mrb, tiny_bignum, "clone", mrb_bignum_clone, MRB_ARGS_NONE());
  mrb_define_method(mrb, tiny_bignum, "to_big", mrb_bignum_to_big, MRB_ARGS_NONE());

  mrb_define_method(mrb, fixnum, "to_big",   fixnum_to_big, MRB_ARGS_NONE());
  mrb_define_method(mrb, string, "to_big",   string_to_big, MRB_ARGS_OPT(1));
}

void mrb_mruby_tiny_bignum_gem_final(mrb_state *mrb)
{
}
