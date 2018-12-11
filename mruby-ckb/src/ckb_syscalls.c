#ifdef CKB_HAS_SYSCALLS

#include <ckb_syscalls.h>

#else

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_FILENAME_LENGTH 256

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ckb_consts.h>

#ifdef MRB_DISABLE_STDIO
#error "stdio must be available to fake syscalls!"
#endif

int ckb_load_tx(void* addr, uint64_t* len, size_t offset)
{
  FILE *fp = fopen("data/tx", "rb");
  if (fp == NULL) {
    return CKB_ITEM_MISSING;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  size_t left_size = size - offset;
  size_t read_size = MIN(left_size, *len);

  fseek(fp, offset, SEEK_SET);
  fread(addr, 1, read_size, fp);
  *len = left_size;

  return CKB_SUCCESS;
}

int ckb_load_cell(void* addr, uint64_t* len, size_t offset, size_t index, size_t source)
{
  char internal_buffer[MAX_FILENAME_LENGTH];
  snprintf(internal_buffer, MAX_FILENAME_LENGTH, "data/cells/%ld/%ld", index, source);

  FILE *fp = fopen(internal_buffer, "rb");
  if (fp == NULL) {
    return CKB_ITEM_MISSING;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  size_t left_size = size - offset;
  size_t read_size = MIN(left_size, *len);

  fseek(fp, offset, SEEK_SET);
  fread(addr, 1, read_size, fp);
  *len = left_size;

  return CKB_SUCCESS;
}

int ckb_load_cell_by_field(void* addr, uint64_t* len, size_t offset, size_t index, size_t source, size_t field)
{
  char internal_buffer[MAX_FILENAME_LENGTH];
  snprintf(internal_buffer, MAX_FILENAME_LENGTH, "data/cell_fields/%ld/%ld/%ld", index, source, field);

  FILE *fp = fopen(internal_buffer, "rb");
  if (fp == NULL) {
    return CKB_ITEM_MISSING;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  size_t left_size = size - offset;
  size_t read_size = MIN(left_size, *len);

  fseek(fp, offset, SEEK_SET);
  fread(addr, 1, read_size, fp);
  *len = left_size;

  return CKB_SUCCESS;
}

int ckb_load_input_by_field(void* addr, uint64_t* len, size_t offset, size_t index, size_t source, size_t field)
{
  char internal_buffer[MAX_FILENAME_LENGTH];
  snprintf(internal_buffer, MAX_FILENAME_LENGTH, "data/input_fields/%ld/%ld/%ld", index, source, field);

  FILE *fp = fopen(internal_buffer, "rb");
  if (fp == NULL) {
    return CKB_ITEM_MISSING;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  size_t left_size = size - offset;
  size_t read_size = MIN(left_size, *len);

  fseek(fp, offset, SEEK_SET);
  fread(addr, 1, read_size, fp);
  *len = left_size;

  return CKB_SUCCESS;
}

int ckb_debug(const char* s)
{
  return puts(s);
}

#endif
