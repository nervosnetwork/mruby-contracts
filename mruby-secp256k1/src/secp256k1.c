#include "mruby.h"
#include "mruby/value.h"

#ifdef SECP256K1_CUSTOM_FUNCS
#define CUSTOM_ABORT 1
#define CUSTOM_PRINT_ERR 1
extern void custom_abort();
extern int custom_print_err(const char * arg, ...);
#endif

#include <secp256k1_static.h>
#include <secp256k1.c>

#ifdef SECP256K1_DISABLE_SIGNING
#ifdef SECP256K1_DISABLE_VERIFYING
#error "Please enable at least one of signing or verifying part!"
#endif
#endif

#ifndef SECP256K1_DISABLE_SIGNING
static mrb_value
secp256k1_mrb_pubkey(mrb_state *mrb, mrb_value obj)
{
  char *secretkey_buf;
  size_t secretkey_len;

  mrb_get_args(mrb, "s", &secretkey_buf, &secretkey_len);

  if (secretkey_len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secret key length is not 32 bytes!");
  }

  secp256k1_context context;
  int ret = secp256k1_context_initialize(&context, SECP256K1_CONTEXT_SIGN);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 initialization failure!");
  }

  secp256k1_pubkey pubkey;
  ret = secp256k1_ec_pubkey_create(&context, &pubkey, secretkey_buf);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 pubkey creation failure!");
  }

  unsigned char buf[256];
  size_t len = 256;
  ret = secp256k1_ec_pubkey_serialize(&context, buf, &len, &pubkey, SECP256K1_EC_COMPRESSED);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 pubkey serializing failure!");
  }

  return mrb_str_new(mrb, buf, len);
}

static mrb_value
secp256k1_mrb_sign(mrb_state *mrb, mrb_value obj)
{
  char *secretkey_buf, *message_buf;
  size_t secretkey_len, message_len;

  mrb_get_args(mrb, "ss", &secretkey_buf, &secretkey_len,
               &message_buf, &message_len);

  if (secretkey_len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secret key length is not 32 bytes!");
  }
  if (message_len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "message length is not 32 bytes!");
  }

  secp256k1_context context;
  int ret = secp256k1_context_initialize(&context, SECP256K1_CONTEXT_SIGN);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 initialization failure!");
  }

  secp256k1_ecdsa_signature signature;
  ret = secp256k1_ecdsa_sign(&context, &signature, message_buf, secretkey_buf,
                             NULL, NULL);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 signing failure!");
  }

  unsigned char buf[256];
  size_t len = 256;
  ret = secp256k1_ecdsa_signature_serialize_der(&context, buf, &len, &signature);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 signature serializing failure!");
  }

  return mrb_str_new(mrb, buf, len);
}
#endif

#ifndef SECP256K1_DISABLE_VERIFYING
static mrb_value
secp256k1_mrb_verify(mrb_state *mrb, mrb_value obj)
{
  char *pubkey_buf, *signature_buf, *message_buf;
  mrb_int pubkey_len, signature_len, message_len;

  mrb_get_args(mrb, "sss", &pubkey_buf, &pubkey_len,
               &signature_buf, &signature_len,
               &message_buf, &message_len);

  if (message_len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "message length is not 32 bytes!");
  }

  secp256k1_context context;
  int ret = secp256k1_context_initialize(&context, SECP256K1_CONTEXT_VERIFY);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 initialization failure!");
  }

  secp256k1_pubkey pubkey;
  ret = secp256k1_ec_pubkey_parse(&context, &pubkey, pubkey_buf, pubkey_len);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 pubkey parsing failure!");
  }

  secp256k1_ecdsa_signature signature;
  ret = secp256k1_ecdsa_signature_parse_der(&context, &signature, signature_buf,
                                            signature_len);
  if (ret == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "secp256k1 signature parsing failure!");
  }

  ret = secp256k1_ecdsa_verify(&context, &signature, message_buf, &pubkey);
  secp256k1_context_deinitialize(&context);

  if (ret == 1) {
    return mrb_true_value();
  } else {
    return mrb_false_value();
  }
}
#endif

void mrb_mruby_secp256k1_gem_init(mrb_state* mrb)
{
  struct RClass *mrb_secp256k1;
  mrb_secp256k1 = mrb_define_module(mrb, "Secp256k1");
#ifndef SECP256K1_DISABLE_SIGNING
  mrb_define_module_function(mrb, mrb_secp256k1, "pubkey", secp256k1_mrb_pubkey, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mrb_secp256k1, "sign", secp256k1_mrb_sign, MRB_ARGS_REQ(2));
#endif

#ifndef SECP256K1_DISABLE_VERIFYING
  mrb_define_module_function(mrb, mrb_secp256k1, "verify", secp256k1_mrb_verify, MRB_ARGS_REQ(3));
#endif
}

void mrb_mruby_secp256k1_gem_final(mrb_state* mrb)
{
}
