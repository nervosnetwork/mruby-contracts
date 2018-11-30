#ifdef CKB_HAS_SYSCALLS

#include <ckb_syscalls.h>

#else

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_FILENAME_LENGTH 256

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MRB_DISABLE_STDIO
#error "stdio must be available to fake syscalls!"
#endif

int ckb_mmap_tx(void* addr, uint64_t* len, unsigned mod, size_t offset)
{
  FILE *fp = fopen("data/tx", "rb");
  if (fp == NULL) {
    return 2;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (mod == 0) {
    if (*len < size) {
      *len = size;
      fclose(fp);
      return 1;
    } else {
      *len = fread(addr, 1, size, fp);
      fclose(fp);
      return 0;
    }
  } else {
    size_t left_size = size - offset;
    size_t read_size = MIN(left_size, *len);

    fseek(fp, offset, SEEK_SET);
    fread(addr, 1, read_size, fp);
    *len = left_size;

    return 0;
  }
}

int ckb_mmap_cell(void* addr, uint64_t* len, unsigned mod, size_t offset, size_t index, size_t source)
{
  char internal_buffer[MAX_FILENAME_LENGTH];
  snprintf(internal_buffer, MAX_FILENAME_LENGTH, "data/cells/%ld/%ld", index, source);

  FILE *fp = fopen(internal_buffer, "rb");
  if (fp == NULL) {
    return 2;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (mod == 0) {
    if (*len < size) {
      *len = size;
      fclose(fp);
      return 1;
    } else {
      *len = fread(addr, 1, size, fp);
      fclose(fp);
      return 0;
    }
  } else {
    size_t left_size = size - offset;
    size_t read_size = MIN(left_size, *len);

    fseek(fp, offset, SEEK_SET);
    fread(addr, 1, read_size, fp);
    *len = left_size;

    return 0;
  }
}

int ckb_mmap_fetch_script_hash(void* addr, uint64_t* len, size_t index, size_t source, size_t category)
{
  char internal_buffer[MAX_FILENAME_LENGTH];
  snprintf(internal_buffer, MAX_FILENAME_LENGTH, "data/script_hashes/%ld/%ld/%ld", index, source, category);

  FILE *fp = fopen(internal_buffer, "rb");
  if (fp == NULL) {
    return 2;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (*len < size) {
    *len = size;
    fclose(fp);
    return 1;
  } else {
    *len = fread(addr, 1, size, fp);
    fclose(fp);
    return 0;
  }
}

int ckb_mmap_fetch_current_script_hash(void* addr, uint64_t* len)
{
  FILE *fp = fopen("data/current_script_hash", "rb");
  if (fp == NULL) {
    return 2;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if (*len < size) {
    *len = size;
    fclose(fp);
    return 1;
  } else {
    *len = fread(addr, 1, size, fp);
    fclose(fp);
    return 0;
  }
}

int ckb_debug(const char* s)
{
  return puts(s);
}

#endif
