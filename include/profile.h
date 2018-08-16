#pragma once
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/perf_event.h>
#include <asm/perf_regs.h>
#include <asm/unistd.h>
#include <perfmon/pfmlib_perf_event.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

struct __attribute__ ((__packed__)) sample {
    uint64_t addr;
};

typedef struct profile_thread {
  pthread_t id;
  pthread_mutex_t mtx;

  /* For perf */
  size_t size;
  struct perf_event_attr *pe;
  struct perf_event_mmap_page *metadata;
  int fd;

  /* For libpfm */
  pfm_perf_encode_arg_t *pfm;
  size_t page_size;
} profile_thread;

void sh_start_profile_thread();
void sh_stop_profile_thread();
void *sh_profile_thread(void *);