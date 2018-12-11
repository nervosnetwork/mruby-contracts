#include "mruby.h"
#include "mruby/array.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include "ckb_consts.h"
#include "protocol_reader.h"

#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(Ckb_Protocol, x)

extern int ckb_load_tx(void* addr, uint64_t* len, size_t offset);
extern int ckb_load_cell(void* addr, uint64_t* len, size_t offset,
                         size_t index, size_t source);
extern int ckb_load_cell_by_field(void* addr, uint64_t* len, size_t offset,
                           size_t index, size_t source, size_t field);
extern int ckb_load_input_by_field(void* addr, uint64_t* len, size_t offset,
                            size_t index, size_t source, size_t field);
extern int ckb_debug(const char* s);


static mrb_value
bytes_to_string(ns(Bytes_table_t) bytes, mrb_state *mrb)
{
  flatbuffers_uint8_vec_t seq = ns(Bytes_seq(bytes));
  size_t len = flatbuffers_uint8_vec_len(seq);

  if (len != 32) {
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
  if (ckb_load_tx(0, &len, 0) != CKB_SUCCESS) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong load tx return value!");
  }

  void* addr = malloc(len);
  if (addr == NULL) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "not enough memory!");
  }

  if (ckb_load_tx(addr, &len, 0) != CKB_SUCCESS) {
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

  mrb_get_args(mrb, "iii", &index, &source, &category);

  if (category != CKB_CELL_FIELD_LOCK_HASH &&
      category != CKB_CELL_FIELD_CONTRACT_HASH) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid category argument!");
  }

  len = 32;
  s = mrb_str_new_capa(mrb, len);
  ret = ckb_load_cell_by_field(RSTRING_PTR(s), &len, 0, index, source, category);
  if (len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "buffer length is not enough!");
  } else if (ret == CKB_ITEM_MISSING) {
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
ckb_mrb_cell_internal_read(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index, len, offset;
  mrb_value buf;
  uint64_t buf_len;
  void* p;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    if (ckb_load_cell(NULL, &buf_len, 0, index, source) != CKB_SUCCESS) {
      buf_len = 0;
    }

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    if (ckb_load_cell(RSTRING_PTR(buf), &buf_len, offset, index, source) != CKB_SUCCESS) {
      return mrb_nil_value();
    }
    RSTR_SET_LEN(mrb_str_ptr(buf), buf_len);

    return buf;
  }
}

static mrb_value
ckb_mrb_cell_field_internal_read(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index, len, offset, field;
  mrb_value buf;
  uint64_t buf_len;
  void* p;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));
  field = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@field")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    if (ckb_load_cell_by_field(NULL, &buf_len, 0, index, source, field) != CKB_SUCCESS) {
      buf_len = 0;
    }

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    if (ckb_load_cell_by_field(RSTRING_PTR(buf), &buf_len, offset, index, source, field) != CKB_SUCCESS) {
      return mrb_nil_value();
    }
    RSTR_SET_LEN(mrb_str_ptr(buf), buf_len);

    return buf;
  }
}

static mrb_value
ckb_mrb_input_field_internal_read(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index, len, offset, field;
  mrb_value buf;
  uint64_t buf_len;
  void* p;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));
  field = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@field")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    if (ckb_load_input_by_field(NULL, &buf_len, 0, index, source, field) != CKB_SUCCESS) {
      buf_len = 0;
    }

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    if (ckb_load_input_by_field(RSTRING_PTR(buf), &buf_len, offset, index, source, field) != CKB_SUCCESS) {
      return mrb_nil_value();
    }
    RSTR_SET_LEN(mrb_str_ptr(buf), buf_len);

    return buf;
  }
}

void
mrb_mruby_ckb_gem_init(mrb_state* mrb)
{
  struct RClass *mrb_ckb, *reader, *cell, *cell_field, *input_field;
  mrb_ckb = mrb_define_module(mrb, "CKB");
  mrb_define_module_function(mrb, mrb_ckb, "load_tx", ckb_mrb_load_tx, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mrb_ckb, "load_script_hash", ckb_mrb_load_script_hash, MRB_ARGS_REQ(3));
  mrb_define_module_function(mrb, mrb_ckb, "debug", ckb_mrb_debug, MRB_ARGS_REQ(1));
  reader = mrb_define_class_under(mrb, mrb_ckb, "Reader", mrb->object_class);
  cell = mrb_define_class_under(mrb, mrb_ckb, "Cell", reader);
  mrb_define_method(mrb, cell, "internal_read", ckb_mrb_cell_internal_read, MRB_ARGS_REQ(1));
  cell_field = mrb_define_class_under(mrb, mrb_ckb, "CellField", reader);
  mrb_define_method(mrb, cell_field, "internal_read", ckb_mrb_cell_field_internal_read, MRB_ARGS_REQ(1));
  input_field = mrb_define_class_under(mrb, mrb_ckb, "InputField", reader);
  mrb_define_method(mrb, cell_field, "internal_read", ckb_mrb_input_field_internal_read, MRB_ARGS_REQ(1));
}

void
mrb_mruby_ckb_gem_final(mrb_state* mrb)
{
}
