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
h256_to_string(ns(H256_struct_t) h256, mrb_state *mrb)
{
  mrb_value s = mrb_str_new_capa(mrb, 32);
  RSTRING_PTR(s)[0] = ns(H256_u0(h256));
  RSTRING_PTR(s)[1] = ns(H256_u1(h256));
  RSTRING_PTR(s)[2] = ns(H256_u2(h256));
  RSTRING_PTR(s)[3] = ns(H256_u3(h256));
  RSTRING_PTR(s)[4] = ns(H256_u4(h256));
  RSTRING_PTR(s)[5] = ns(H256_u5(h256));
  RSTRING_PTR(s)[6] = ns(H256_u6(h256));
  RSTRING_PTR(s)[7] = ns(H256_u7(h256));
  RSTRING_PTR(s)[8] = ns(H256_u8(h256));
  RSTRING_PTR(s)[9] = ns(H256_u9(h256));
  RSTRING_PTR(s)[10] = ns(H256_u10(h256));
  RSTRING_PTR(s)[11] = ns(H256_u11(h256));
  RSTRING_PTR(s)[12] = ns(H256_u12(h256));
  RSTRING_PTR(s)[13] = ns(H256_u13(h256));
  RSTRING_PTR(s)[14] = ns(H256_u14(h256));
  RSTRING_PTR(s)[15] = ns(H256_u15(h256));
  RSTRING_PTR(s)[16] = ns(H256_u16(h256));
  RSTRING_PTR(s)[17] = ns(H256_u17(h256));
  RSTRING_PTR(s)[18] = ns(H256_u18(h256));
  RSTRING_PTR(s)[19] = ns(H256_u19(h256));
  RSTRING_PTR(s)[20] = ns(H256_u20(h256));
  RSTRING_PTR(s)[21] = ns(H256_u21(h256));
  RSTRING_PTR(s)[22] = ns(H256_u22(h256));
  RSTRING_PTR(s)[23] = ns(H256_u23(h256));
  RSTRING_PTR(s)[24] = ns(H256_u24(h256));
  RSTRING_PTR(s)[25] = ns(H256_u25(h256));
  RSTRING_PTR(s)[26] = ns(H256_u26(h256));
  RSTRING_PTR(s)[27] = ns(H256_u27(h256));
  RSTRING_PTR(s)[28] = ns(H256_u28(h256));
  RSTRING_PTR(s)[29] = ns(H256_u29(h256));
  RSTRING_PTR(s)[30] = ns(H256_u30(h256));
  RSTRING_PTR(s)[31] = ns(H256_u31(h256));
  RSTR_SET_LEN(mrb_str_ptr(s), 32);
  return s;
}

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
               h256_to_string(ns(OutPoint_hash(outpoint)), mrb));
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "index"),
               mrb_fixnum_value(ns(OutPoint_index(outpoint))));
  return v;
}

static mrb_value
script_to_value(ns(Script_table_t) script, mrb_state *mrb)
{
  mrb_value v = mrb_hash_new(mrb);
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "version"),
               mrb_fixnum_value(ns(Script_version(script))));

  ns(Bytes_vec_t) args = ns(Script_args(script));
  size_t args_len = ns(Bytes_vec_len(args));
  mrb_value margs = mrb_ary_new_capa(mrb, args_len);
  for (int i = 0; i < args_len; i++) {
    mrb_ary_push(mrb, margs, bytes_to_string(ns(Bytes_vec_at(args, i)), mrb));
  }
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "args"), margs);

  ns(Bytes_table_t) binary = ns(Script_binary(script));
  if (binary) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "binary"),
                 bytes_to_string(binary, mrb));
  }

  ns(H256_struct_t) reference = ns(Script_reference(script));
  if (reference) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "reference"),
                 h256_to_string(reference, mrb));
  }

  ns(Bytes_vec_t) signed_args = ns(Script_signed_args(script));
  size_t signed_args_len = ns(Bytes_vec_len(signed_args));
  mrb_value msigned_args = mrb_ary_new_capa(mrb, signed_args_len);
  for (int i = 0; i < signed_args_len; i++) {
    mrb_ary_push(mrb, msigned_args, bytes_to_string(ns(Bytes_vec_at(signed_args, i)), mrb));
  }
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "signed_args"), msigned_args);

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
                 h256_to_string(ns(CellInput_hash(input)), mrb));
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
                 h256_to_string(ns(CellOutput_lock(output)), mrb));

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
      category != CKB_CELL_FIELD_TYPE_HASH) {
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
ckb_mrb_load_output_type_script(mrb_state *mrb, mrb_value obj)
{
  mrb_int index;
  uint64_t len;
  mrb_value v;
  int ret;
  void *addr = NULL;

  mrb_get_args(mrb, "i", &index);

  len = 0;
  ret = ckb_load_cell_by_field(NULL, &len, 0, index, CKB_SOURCE_OUTPUT, CKB_CELL_FIELD_TYPE);
  if (ret == CKB_ITEM_MISSING) {
    return mrb_nil_value();
  }
  if (ret != CKB_SUCCESS) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong load cell by field return value!");
  }

  addr = malloc(len);
  if (addr == NULL) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "not enough memory!");
  }

  ret = ckb_load_cell_by_field(addr, &len, 0, index, CKB_SOURCE_OUTPUT, CKB_CELL_FIELD_TYPE);
  if (ret != CKB_SUCCESS) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong load cell by field return value!");
  }

  ns(Script_table_t) script;
  if (!(script = ns(Script_as_root(addr)))) {
    free(addr);
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error parsing script!");
  }

  v = script_to_value(script, mrb);
  free(addr);
  return v;
}

static mrb_value
ckb_mrb_load_input_unlock_script(mrb_state *mrb, mrb_value obj)
{
  mrb_int index;
  uint64_t len;
  mrb_value v;
  int ret;
  void *addr = NULL;

  mrb_get_args(mrb, "i", &index);

  len = 0;
  ret = ckb_load_input_by_field(NULL, &len, 0, index, CKB_SOURCE_INPUT, CKB_INPUT_FIELD_UNLOCK);
  if (ret == CKB_ITEM_MISSING) {
    return mrb_nil_value();
  }
  if (ret != CKB_SUCCESS) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong load cell by field return value!");
  }

  addr = malloc(len);
  if (addr == NULL) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "not enough memory!");
  }

  ret = ckb_load_input_by_field(addr, &len, 0, index, CKB_SOURCE_INPUT, CKB_INPUT_FIELD_UNLOCK);
  if (ret != CKB_SUCCESS) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong load cell by field return value!");
  }

  ns(Script_table_t) script;
  if (!(script = ns(Script_as_root(addr)))) {
    free(addr);
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error parsing script!");
  }

  v = script_to_value(script, mrb);
  free(addr);
  return v;
}

static mrb_value
ckb_mrb_load_input_out_point(mrb_state *mrb, mrb_value obj)
{
  mrb_int index, source;
  uint64_t len = 0;
  void* addr;

  mrb_get_args(mrb, "ii", &index, &source);

  if (ckb_load_input_by_field(0, &len, 0, index, source, CKB_INPUT_FIELD_OUT_POINT) == CKB_ITEM_MISSING) {
    return mrb_nil_value();
  }

  addr = malloc(len);
  if (addr == NULL) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "not enough memory!");
  }

  if (ckb_load_input_by_field(addr, &len, 0, index, source, CKB_INPUT_FIELD_OUT_POINT) != CKB_SUCCESS) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error loading input outpoint!");
  }

  ns(OutPoint_table_t) op;
  if (!(op = ns(OutPoint_as_root(addr)))) {
    free(addr);
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error parsing outpoint!");
  }

  mrb_value o = outpoint_to_value(op, mrb);

  free(addr);
  return o;
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
    RSTR_SET_LEN(mrb_str_ptr(buf), len);

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
    RSTR_SET_LEN(mrb_str_ptr(buf), len);

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

    return mrb_fixnum_value(len);
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
  mrb_define_module_function(mrb, mrb_ckb, "load_output_type_script", ckb_mrb_load_output_type_script, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mrb_ckb, "load_input_unlock_script", ckb_mrb_load_input_unlock_script, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mrb_ckb, "load_input_out_point", ckb_mrb_load_input_out_point, MRB_ARGS_REQ(2));
  mrb_define_module_function(mrb, mrb_ckb, "debug", ckb_mrb_debug, MRB_ARGS_REQ(1));
  reader = mrb_define_class_under(mrb, mrb_ckb, "Reader", mrb->object_class);
  cell = mrb_define_class_under(mrb, mrb_ckb, "Cell", reader);
  mrb_define_method(mrb, cell, "internal_read", ckb_mrb_cell_internal_read, MRB_ARGS_REQ(1));
  cell_field = mrb_define_class_under(mrb, mrb_ckb, "CellField", reader);
  mrb_define_method(mrb, cell_field, "internal_read", ckb_mrb_cell_field_internal_read, MRB_ARGS_REQ(1));
  input_field = mrb_define_class_under(mrb, mrb_ckb, "InputField", reader);
  mrb_define_method(mrb, input_field, "internal_read", ckb_mrb_input_field_internal_read, MRB_ARGS_REQ(1));
}

void
mrb_mruby_ckb_gem_final(mrb_state* mrb)
{
}
