#include "mruby.h"
#include "mruby/array.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include "protocol_reader.h"

#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(Ckb_Protocol, x)

#define SUCCESS 0
#define OVERRIDE_LEN 1
#define ITEM_MISSING 2

extern int ckb_mmap_tx(void* addr, uint64_t* len, unsigned mod, size_t offset);
extern int ckb_mmap_cell(void* addr, uint64_t* len, unsigned mod, size_t offset, size_t index, size_t source);
extern int ckb_mmap_fetch_script_hash(void* addr, uint64_t* len, size_t index, size_t source, size_t category);
extern int ckb_debug(const char* s);

static mrb_value
bytes_to_string(ns(Bytes_table_t) bytes, mrb_state *mrb)
{
  flatbuffers_uint8_vec_t seq = ns(Bytes_seq(bytes));
  size_t len = flatbuffers_uint8_vec_len(seq);

  if (len != 256) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong bytes length!");
  }

  mrb_value s = mrb_str_new_capa(mrb, len);
  for (int i = 0; i < len; i++) {
    RSTRING_PTR(s)[i] = flatbuffers_uint8_vec_at(seq, i);
  }
  RSTR_SET_LEN(mrb_str_ptr(s), len);
  return s;
}

static mrb_value
outpoint_to_value(ns(OutPoint_table_t) outpoint, mrb_state *mrb)
{
  mrb_value v = mrb_hash_new(mrb);
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "hash"),
               bytes_to_string(ns(OutPoint_hash(outpoint)), mrb));
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "index"),
               mrb_fixnum_value(ns(OutPoint_index(outpoint))));
  return v;
}

static mrb_value
ckb_mrb_load_tx(mrb_state *mrb, mrb_value obj)
{
  uint64_t len = 0;
  if (ckb_mmap_tx(0, &len, 0, 0) != OVERRIDE_LEN) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong load tx return value!");
  }

  void* addr = malloc(len);
  if (addr == NULL) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "not enough memory!");
  }

  if (ckb_mmap_tx(addr, &len, 0, 0) != SUCCESS) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error loading tx!");
  }

  ns(Transaction_table_t) tx;
  if (!(tx = ns(Transaction_as_root(addr)))) {
    free(addr);
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error parsing tx data!");
  }

  mrb_value mtx = mrb_hash_new(mrb);
  mrb_hash_set(mrb, mtx, mrb_str_new_lit(mrb, "version"),
               mrb_fixnum_value(ns(Transaction_version(tx))));

  ns(OutPoint_vec_t) deps = ns(Transaction_deps(tx));
  size_t deps_len = ns(OutPoint_vec_len(deps));
  mrb_value mdeps = mrb_ary_new_capa(mrb, deps_len);
  for (int i = 0; i < deps_len; i++) {
    mrb_ary_push(mrb, mdeps, outpoint_to_value(ns(OutPoint_vec_at(deps, i)), mrb));
  }
  mrb_hash_set(mrb, mtx, mrb_str_new_lit(mrb, "deps"), mdeps);

  ns(CellInput_vec_t) inputs = ns(Transaction_inputs(tx));
  size_t inputs_len = ns(CellInput_vec_len(inputs));
  mrb_value minputs = mrb_ary_new_capa(mrb, inputs_len);
  for (int i = 0; i < inputs_len; i++) {
    ns(CellInput_table_t) input = ns(CellInput_vec_at(inputs, i));
    mrb_value minput = mrb_hash_new(mrb);
    mrb_hash_set(mrb, minput, mrb_str_new_lit(mrb, "hash"),
                 bytes_to_string(ns(CellInput_hash(input)), mrb));
    mrb_hash_set(mrb, minput, mrb_str_new_lit(mrb, "index"),
                 mrb_fixnum_value(ns(CellInput_index(input))));

    ns(Script_table_t) script = ns(CellInput_unlock(input));
    mrb_value mscript = mrb_hash_new(mrb);
    mrb_hash_set(mrb, mscript, mrb_str_new_lit(mrb, "version"),
                 mrb_fixnum_value(ns(Script_version(script))));
    ns(Bytes_vec_t) arguments = ns(Script_arguments(script));
    size_t arguments_len = ns(Bytes_vec_len(arguments));
    mrb_value marguments = mrb_ary_new_capa(mrb, arguments_len);
    for (int j = 0; j < arguments_len; j++) {
      mrb_ary_push(mrb, marguments, bytes_to_string(ns(Bytes_vec_at(arguments, j)), mrb));
    }
    mrb_hash_set(mrb, mscript, mrb_str_new_lit(mrb, "arguments"), marguments);
    mrb_hash_set(mrb, mscript, mrb_str_new_lit(mrb, "redeem_script"),
                 bytes_to_string(ns(Script_redeem_script(script)), mrb));
    mrb_hash_set(mrb, mscript, mrb_str_new_lit(mrb, "redeem_reference"),
                 outpoint_to_value(ns(Script_redeem_reference(script)), mrb));
    ns(Bytes_vec_t) redeem_arguments = ns(Script_redeem_arguments(script));
    size_t redeem_arguments_len = ns(Bytes_vec_len(redeem_arguments));
    mrb_value mredeem_arguments = mrb_ary_new_capa(mrb, redeem_arguments_len);
    for (int j = 0; j < redeem_arguments_len; j++) {
      mrb_ary_push(mrb, mredeem_arguments,
                   bytes_to_string(ns(Bytes_vec_at(redeem_arguments, j)), mrb));
    }
    mrb_hash_set(mrb, mscript, mrb_str_new_lit(mrb, "redeem_arguments"),
                 mredeem_arguments);

    mrb_hash_set(mrb, minput, mrb_str_new_lit(mrb, "unlock"), mscript);
    mrb_ary_push(mrb, minputs, minput);
  }
  mrb_hash_set(mrb, mtx, mrb_str_new_lit(mrb, "inputs"), minputs);

  ns(CellOutput_vec_t) outputs = ns(Transaction_outputs(tx));
  size_t outputs_len = ns(CellOutput_vec_len(outputs));
  mrb_value moutputs = mrb_ary_new_capa(mrb, outputs_len);
  for (int i = 0; i < outputs_len; i++) {
    ns(CellOutput_table_t) output = ns(CellOutput_vec_at(outputs, i));
    mrb_value moutput = mrb_hash_new(mrb);
    mrb_hash_set(mrb, moutput, mrb_str_new_lit(mrb, "capacity"),
                 mrb_fixnum_value(ns(CellOutput_capacity(output))));
    mrb_hash_set(mrb, moutput, mrb_str_new_lit(mrb, "lock"),
                 bytes_to_string(ns(CellOutput_lock(output)), mrb));

    mrb_ary_push(mrb, moutputs, moutput);
  }
  mrb_hash_set(mrb, mtx, mrb_str_new_lit(mrb, "outputs"), moutputs);

  free(addr);
  return mtx;
}

static mrb_value
ckb_mrb_load_script_hash(mrb_state *mrb, mrb_value obj)
{
  mrb_int index, source, category;
  uint64_t len;
  mrb_value s;
  int ret;

  mrb_get_args(mrb, "ii", &index, &source, &category);

  len = 32;
  s = mrb_str_new_capa(mrb, len);
  ret = ckb_mmap_fetch_script_hash(RSTRING_PTR(s), &len, index, source, category);
  if (ret == OVERRIDE_LEN) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "buffer length is not enough!");
  } else if (ret == ITEM_MISSING) {
    return mrb_nil_value();
  }
  RSTR_SET_LEN(mrb_str_ptr(s), len);
  return s;
}

static mrb_value
ckb_mrb_debug(mrb_state *mrb, mrb_value obj)
{
  mrb_value s;

  mrb_get_args(mrb, "S", &s);

  const char *cstr = mrb_string_value_cstr(mrb, &s);
  ckb_debug(cstr);

  return obj;
}

static mrb_value
ckb_mrb_cell_length(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index;
  uint64_t len;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  len = 0;
  if (ckb_mmap_cell(NULL, &len, 0, 0, index, source) != 1) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid response from mmap cell!");
  }

  return mrb_fixnum_value(len);
}

static mrb_value
ckb_mrb_cell_read(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index, offset, length;
  mrb_value buf;
  uint64_t len;

  mrb_get_args(mrb, "ii", &offset, &length);

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  buf = mrb_str_new_capa(mrb, length);
  len = length;
  if (ckb_mmap_cell(RSTRING_PTR(buf), &len, 1, offset, index, source) != 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid response from mmap cell!");
  }
  RSTR_SET_LEN(mrb_str_ptr(buf), len);

  return buf;
}

static mrb_value
ckb_mrb_cell_readall(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index;
  mrb_value buf;
  uint64_t len;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  len = 0;
  if (ckb_mmap_cell(NULL, &len, 0, 0, index, source) != 1) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid response from mmap cell!");
  }

  buf = mrb_str_new_capa(mrb, len);
  if (ckb_mmap_cell(RSTRING_PTR(buf), &len, 0, 0, index, source) != 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid response from mmap cell!");
  }
  RSTR_SET_LEN(mrb_str_ptr(buf), len);

  return buf;
}

void
mrb_mruby_ckb_gem_init(mrb_state* mrb)
{
  struct RClass *mrb_ckb, *cell;
  mrb_ckb = mrb_define_module(mrb, "CKB");
  mrb_define_module_function(mrb, mrb_ckb, "load_tx", ckb_mrb_load_tx, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mrb_ckb, "load_script_hash", ckb_mrb_load_script_hash, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, mrb_ckb, "debug", ckb_mrb_debug, MRB_ARGS_REQ(1));
  cell = mrb_define_class_under(mrb, mrb_ckb, "Cell", mrb->object_class);
  mrb_define_method(mrb, cell, "length", ckb_mrb_cell_length, MRB_ARGS_NONE());
  mrb_define_method(mrb, cell, "read", ckb_mrb_cell_read, MRB_ARGS_REQ(2));
  mrb_define_method(mrb, cell, "readall", ckb_mrb_cell_readall, MRB_ARGS_NONE());
}

void
mrb_mruby_ckb_gem_final(mrb_state* mrb)
{
}
