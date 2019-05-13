#include "mruby.h"
#include "mruby/array.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"

#include "ckb_consts.h"
#include "protocol_reader.h"

#undef ns
#define ns(x) FLATBUFFERS_WRAP_NAMESPACE(Ckb_Protocol, x)

#define PROCESS_LOAD_RESULT(ret)             \
  switch (ret) {                             \
    case CKB_SUCCESS:                        \
      break;                                 \
    case CKB_INDEX_OUT_OF_BOUND:             \
      raise_index_out_of_bound(mrb);         \
      break;                                 \
    case CKB_ITEM_MISSING:                   \
      return mrb_nil_value();                \
    default:                                 \
      mrb_raisef(mrb, E_ARGUMENT_ERROR,      \
                 "Invalid return value: %S", \
                 mrb_fixnum_value(ret));     \
      break;                                 \
    }


extern int ckb_load_tx_hash(void* addr, uint64_t* len, size_t offset);
extern int ckb_load_script_hash(void* addr, uint64_t* len, size_t offset);
extern int ckb_load_cell(void* addr, uint64_t* len, size_t offset,
                         size_t index, size_t source);
extern int ckb_load_input(void* addr, uint64_t* len, size_t offset,
                          size_t index, size_t source);
extern int ckb_load_header(void* addr, uint64_t* len, size_t offset,
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
  mrb_value s;
  flatbuffers_uint8_vec_t seq = ns(Bytes_seq(bytes));
  size_t len = flatbuffers_uint8_vec_len(seq);

  if (len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong bytes length!");
  }

  s = mrb_str_new_capa(mrb, len);
  for (int i = 0; i < len; i++) {
    RSTRING_PTR(s)[i] = flatbuffers_uint8_vec_at(seq, i);
  }
  RSTR_SET_LEN(mrb_str_ptr(s), len);
  return s;
}

static mrb_value
outpoint_to_value(ns(OutPoint_table_t) outpoint, mrb_state *mrb)
{
  ns(H256_struct_t) h256;
  mrb_value v = mrb_hash_new(mrb);
  if ((h256 = ns(OutPoint_tx_hash(outpoint))) != NULL) {
    mrb_value cell = mrb_hash_new(mrb);
    mrb_hash_set(mrb, cell, mrb_str_new_lit(mrb, "hash"),
                 h256_to_string(h256, mrb));
    mrb_hash_set(mrb, cell, mrb_str_new_lit(mrb, "index"),
                 mrb_fixnum_value(ns(OutPoint_index(outpoint))));
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "cell"), cell);
  }
  if ((h256 = ns(OutPoint_block_hash(outpoint))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "block_hash"),
                 h256_to_string(h256, mrb));
  }
  return v;
}

static void
raise_index_out_of_bound(mrb_state *mrb)
{
  struct RClass *mrb_ckb = mrb_module_get(mrb, "CKB");
  struct RClass *error_class = mrb_class_get_under(mrb, mrb_ckb, "IndexOutOfBound");
  mrb_raise(mrb, error_class, "Index out of bound!");
}

static mrb_value
ckb_mrb_parse_script(mrb_state *mrb, mrb_value self)
{
  mrb_value s, v, margs;
  ns(Bytes_vec_t) args;
  ns(Script_table_t) script;
  ns(H256_struct_t) code_hash;
  size_t args_len;
  int i;

  mrb_get_args(mrb, "S", &s);

  script = ns(Script_as_root(RSTRING_PTR(s)));
  if (!script) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error parsing script data!");
  }

  v = mrb_hash_new(mrb);
  args = ns(Script_args(script));
  args_len = ns(Bytes_vec_len(args));
  margs = mrb_ary_new_capa(mrb, args_len);
  for (i = 0; i < args_len; i++) {
    mrb_ary_push(mrb, margs, bytes_to_string(ns(Bytes_vec_at(args, i)), mrb));
  }
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "args"), margs);

  code_hash = ns(Script_code_hash(script));
  if (code_hash) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "code_hash"),
                 h256_to_string(code_hash, mrb));
  }

  return v;
}

static mrb_value
ckb_mrb_parse_header(mrb_state *mrb, mrb_value self)
{
  mrb_value s, v;
  ns(Header_table_t) header;
  ns(Bytes_table_t) bytes;
  ns(H256_struct_t) hash;
  size_t args_len;
  int i;

  mrb_get_args(mrb, "S", &s);

  header = ns(Header_as_root(RSTRING_PTR(s)));
  if (!header) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error parsing header data!");
  }

  v = mrb_hash_new(mrb);
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "version"), mrb_fixnum_value(ns(Header_version(header))));
  if ((hash = ns(Header_parent_hash(header))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "parent_hash"), h256_to_string(hash, mrb));
  }
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "timestamp"), mrb_fixnum_value(ns(Header_timestamp(header))));
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "number"), mrb_fixnum_value(ns(Header_number(header))));
  if ((hash = ns(Header_transactions_root(header))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "transactions_root"), h256_to_string(hash, mrb));
  }
  if ((hash = ns(Header_witnesses_root(header))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "witnesses_root"), h256_to_string(hash, mrb));
  }
  if ((hash = ns(Header_proposals_hash(header))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "proposals_hash"), h256_to_string(hash, mrb));
  }
  if ((bytes = ns(Header_difficulty(header))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "difficulty"), bytes_to_string(bytes, mrb));
  }
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "nonce"), mrb_fixnum_value(ns(Header_nonce(header))));
  if ((bytes = ns(Header_proof(header))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "proof"), bytes_to_string(bytes, mrb));
  }
  if ((hash = ns(Header_uncles_hash(header))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "uncles_hash"), h256_to_string(hash, mrb));
  }
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "uncles_count"), mrb_fixnum_value(ns(Header_uncles_count(header))));
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "epoch"), mrb_fixnum_value(ns(Header_epoch(header))));

  return v;
}

static mrb_value
ckb_mrb_parse_input(mrb_state *mrb, mrb_value self)
{
  mrb_value s, v, t;
  ns(Bytes_vec_t) args;
  ns(CellInput_table_t) input;
  ns(H256_struct_t) hash;
  size_t args_len;
  int i;

  mrb_get_args(mrb, "S", &s);

  input = ns(CellInput_as_root(RSTRING_PTR(s)));
  if (!input) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "error parsing input data!");
  }

  v = mrb_hash_new(mrb);
  args = ns(CellInput_args(input));
  args_len = ns(Bytes_vec_len(args));
  t = mrb_ary_new_capa(mrb, args_len);
  for (i = 0; i < args_len; i++) {
    mrb_ary_push(mrb, t, bytes_to_string(ns(Bytes_vec_at(args, i)), mrb));
  }
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "args"), t);

  if ((hash = ns(CellInput_tx_hash(input))) != NULL) {
    mrb_value t = mrb_hash_new(mrb);
    mrb_hash_set(mrb, t, mrb_str_new_lit(mrb, "hash"), h256_to_string(hash, mrb));
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "index"), mrb_fixnum_value(ns(CellInput_index(input))));
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "cell"), t);
  }
  if ((hash = ns(CellInput_block_hash(input))) != NULL) {
    mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "block_hash"), h256_to_string(hash, mrb));
  }
  mrb_hash_set(mrb, v, mrb_str_new_lit(mrb, "since"), mrb_fixnum_value(ns(CellInput_since(input))));

  return v;
}

static mrb_value
ckb_mrb_load_tx_hash(mrb_state *mrb, mrb_value obj)
{
  uint64_t len;
  mrb_value s;
  int ret;

  len = 32;
  s = mrb_str_new_capa(mrb, len);
  ret = ckb_load_tx_hash(RSTRING_PTR(s), &len, 0);
  if (ret != CKB_SUCCESS || len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "cannot load transaction hash!");
  }
  RSTR_SET_LEN(mrb_str_ptr(s), len);
  return s;
}

static mrb_value
ckb_mrb_load_script_hash(mrb_state *mrb, mrb_value obj)
{
  uint64_t len;
  mrb_value s;
  int ret;

  len = 32;
  s = mrb_str_new_capa(mrb, len);
  ret = ckb_load_script_hash(RSTRING_PTR(s), &len, 0);
  if (ret != CKB_SUCCESS || len != 32) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "cannot load script hash!");
  }
  RSTR_SET_LEN(mrb_str_ptr(s), len);
  return s;
}

static mrb_value
ckb_mrb_debug(mrb_state *mrb, mrb_value obj)
{
  mrb_value s;

  mrb_get_args(mrb, "S", &s);

  ckb_debug(mrb_string_value_cstr(mrb, &s));

  return obj;
}

static mrb_value
ckb_mrb_cell_internal_read(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index, len, offset;
  mrb_value buf;
  uint64_t buf_len;
  void* p;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_cell(NULL, &buf_len, 0, index, source);
    PROCESS_LOAD_RESULT(ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_cell(RSTRING_PTR(buf), &buf_len, offset, index, source);
    PROCESS_LOAD_RESULT(ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

static mrb_value
ckb_mrb_input_internal_read(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index, len, offset;
  mrb_value buf;
  uint64_t buf_len;
  void* p;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_input(NULL, &buf_len, 0, index, source);
    PROCESS_LOAD_RESULT(ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_input(RSTRING_PTR(buf), &buf_len, offset, index, source);
    PROCESS_LOAD_RESULT(ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

static mrb_value
ckb_mrb_header_internal_read(mrb_state *mrb, mrb_value self)
{
  mrb_int source, index, len, offset;
  mrb_value buf;
  uint64_t buf_len;
  void* p;
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_header(NULL, &buf_len, 0, index, source);
    PROCESS_LOAD_RESULT(ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_header(RSTRING_PTR(buf), &buf_len, offset, index, source);
    PROCESS_LOAD_RESULT(ret);

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
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));
  field = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@field")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_cell_by_field(NULL, &buf_len, 0, index, source, field);
    PROCESS_LOAD_RESULT(ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_cell_by_field(RSTRING_PTR(buf), &buf_len, offset, index, source, field);
    PROCESS_LOAD_RESULT(ret);

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
  int ret;

  source = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")));
  index = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@index")));
  field = mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@field")));

  mrb_get_args(mrb, "i|i", &len, &offset);

  buf_len = len;
  if (buf_len == 0) {
    ret = ckb_load_input_by_field(NULL, &buf_len, 0, index, source, field);
    PROCESS_LOAD_RESULT(ret);

    return mrb_fixnum_value(buf_len);
  } else {
    buf = mrb_str_new_capa(mrb, buf_len);
    ret = ckb_load_input_by_field(RSTRING_PTR(buf), &buf_len, offset, index, source, field);
    PROCESS_LOAD_RESULT(ret);

    RSTR_SET_LEN(mrb_str_ptr(buf), len);
    return buf;
  }
}

void
mrb_mruby_ckb_gem_init(mrb_state* mrb)
{
  struct RClass *mrb_ckb, *reader, *cell, *input, *header, *cell_field, *input_field;
  mrb_ckb = mrb_define_module(mrb, "CKB");
  mrb_define_module_function(mrb, mrb_ckb, "parse_header", ckb_mrb_parse_header, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mrb_ckb, "parse_input", ckb_mrb_parse_input, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mrb_ckb, "parse_script", ckb_mrb_parse_script, MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, mrb_ckb, "load_tx_hash", ckb_mrb_load_tx_hash, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mrb_ckb, "load_script_hash", ckb_mrb_load_script_hash, MRB_ARGS_NONE());
  mrb_define_module_function(mrb, mrb_ckb, "debug", ckb_mrb_debug, MRB_ARGS_REQ(1));
  reader = mrb_define_class_under(mrb, mrb_ckb, "Reader", mrb->object_class);
  cell = mrb_define_class_under(mrb, mrb_ckb, "Cell", reader);
  mrb_define_method(mrb, cell, "internal_read", ckb_mrb_cell_internal_read, MRB_ARGS_REQ(1));
  input = mrb_define_class_under(mrb, mrb_ckb, "Input", reader);
  mrb_define_method(mrb, input, "internal_read", ckb_mrb_input_internal_read, MRB_ARGS_REQ(1));
  header = mrb_define_class_under(mrb, mrb_ckb, "Header", reader);
  mrb_define_method(mrb, header, "internal_read", ckb_mrb_header_internal_read, MRB_ARGS_REQ(1));
  cell_field = mrb_define_class_under(mrb, mrb_ckb, "CellField", reader);
  mrb_define_method(mrb, cell_field, "internal_read", ckb_mrb_cell_field_internal_read, MRB_ARGS_REQ(1));
  input_field = mrb_define_class_under(mrb, mrb_ckb, "InputField", reader);
  mrb_define_method(mrb, input_field, "internal_read", ckb_mrb_input_field_internal_read, MRB_ARGS_REQ(1));
}

void
mrb_mruby_ckb_gem_final(mrb_state* mrb)
{
}
