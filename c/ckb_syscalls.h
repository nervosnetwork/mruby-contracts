#ifndef CKB_SYSCALLS_H_
#define CKB_SYSCALLS_H_

#include <stdint.h>
#include <stdlib.h>

#define SYS_exit 93
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

#endif  /* CKB_SYSCALLS_H_ */
