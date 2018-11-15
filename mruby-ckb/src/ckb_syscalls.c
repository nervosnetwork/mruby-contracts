#ifdef CKB_HAS_SYSCALLS

#include <stdint.h>
#include <stdlib.h>

#define SYS_ckb_mmap_tx 2049
#define SYS_ckb_mmap_cell 2050
#define SYS_ckb_fetch_script_hash 2051
#define SYS_ckb_debug 2177

static inline long
__internal_syscall(long n, long _a0, long _a1, long _a2, long _a3, long _a4, long _a5)
{
  register long a0 asm("a0") = _a0;
  register long a1 asm("a1") = _a1;
  register long a2 asm("a2") = _a2;
  register long a3 asm("a3") = _a3;
  register long a4 asm("a4") = _a4;
  register long a5 asm("a5") = _a5;

#ifdef __riscv_32e
  register long syscall_id asm("t0") = n;
#else
  register long syscall_id asm("a7") = n;
#endif

  asm volatile ("scall"
		: "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(syscall_id));

  return a0;
}

#define syscall(n, a, b, c, d, e, f) \
        __internal_syscall(n, (long)(a), (long)(b), (long)(c), (long)(d), (long)(e), (long)(f))

int ckb_mmap_tx(void* addr, uint64_t* len, unsigned mod, size_t offset)
{
  return syscall(SYS_ckb_mmap_tx, addr, len, mod, offset, 0, 0);
}

int ckb_mmap_cell(void* addr, uint64_t* len, unsigned mod, size_t offset, size_t index, size_t source)
{
  return syscall(SYS_ckb_mmap_cell, addr, len, mod, offset, index, source);
}

int ckb_mmap_fetch_script_hash(void* addr, uint64_t* len, size_t index, size_t source, size_t category)
{
  return syscall(SYS_ckb_fetch_script_hash, addr, len, index, source, category, 0);
}

int ckb_debug(const char* s)
{
  return syscall(SYS_ckb_debug, s, 0, 0, 0, 0, 0);
}


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

int ckb_debug(const char* s)
{
  return puts(s);
}

#endif
