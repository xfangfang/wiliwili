// PS5 OpenGL - OpenGL implementation for PlayStation 5.
// Copyright (C) 2026 BlackBearReloaded
// SPDX-License-Identifier: GPL-3.0-or-later

/* Shared native-app allocator integration, originally used by the CTS runner. */
#include <errno.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

void *__real_malloc(size_t size);
void *__real_calloc(size_t count, size_t size);
void *__real_realloc(void *address, size_t size);
void __real_free(void *address);
int __real_posix_memalign(void **address, size_t alignment, size_t size);
size_t __real_malloc_usable_size(const void *address);

void *sceLibcMspaceCreate(const char *name, void *base, size_t size,
                          unsigned flags);
void *sceLibcMspaceMalloc(void *mspace, size_t size);
void *sceLibcMspaceCalloc(void *mspace, size_t count, size_t size);
void *sceLibcMspaceRealloc(void *mspace, void *address, size_t size);
void sceLibcMspaceFree(void *mspace, void *address);
int sceLibcMspacePosixMemalign(void *mspace, void **address, size_t alignment,
                              size_t size);
size_t sceLibcMspaceMallocUsableSize(const void *address);
int64_t sceKernelGetDirectMemorySize(void);
int32_t sceKernelAllocateDirectMemory(int64_t search_start, int64_t search_end,
                                     size_t length, size_t alignment,
                                     int memory_type, int64_t *physical_start);
int32_t sceKernelMapDirectMemory(void **address, size_t length, int protection,
                                int flags, int64_t physical_start, size_t alignment);

/* ponytail: 512 MiB process-lifetime heap by default; revisit the budget only
 * for measured application needs. Do not unmap underneath late C++ destructors.
 * A title may define ps5_opengl_heap_size to reserve more (the CTS runner keeps
 * every test case object alive); smaller sizes are tried if mapping fails.
 *
 * The measurement that set the budget: the pictures of the lists are a
 * megabyte each and the video keeps its frames in the same heap. With 128 MiB
 * the heap filled up before a video could show its first frame: the pictures
 * stopped decoding and the video stayed black. */
__attribute__((weak)) const size_t ps5_opengl_heap_size = 1024u * 1024u * 1024u;
/* A title may define ps5_opengl_heap_zero_fill as 1: malloc, posix_memalign and
 * the growth of realloc then return zero-filled memory, so code that reads heap
 * memory it never wrote behaves the same on every run. */
__attribute__((weak)) const int ps5_opengl_heap_zero_fill = 0;
static size_t ps5_heap_size;
#define PS5_OPENGL_HEAP_SIZE ps5_heap_size

static atomic_int ps5_heap_state;
static void *ps5_heap_base;
static void *ps5_heap_mspace;
/* Owned-heap usable bytes only: not GPU mappings, foreign heaps or process RSS.
 * Relaxed snapshots are observations, not an allocator synchronization fence. */
static atomic_size_t ps5_heap_live_bytes, ps5_heap_peak_bytes, ps5_heap_blocks;
static atomic_size_t ps5_heap_failures, ps5_heap_ambiguous_zero_reallocs;

static void ps5_heap_resize_stats(size_t before, size_t after) {
  size_t live = after >= before
      ? atomic_fetch_add_explicit(&ps5_heap_live_bytes, after - before,
                                  memory_order_relaxed) + after - before
      : atomic_fetch_sub_explicit(&ps5_heap_live_bytes, before - after,
                                  memory_order_relaxed) - (before - after);
  size_t peak = atomic_load_explicit(&ps5_heap_peak_bytes, memory_order_relaxed);
  while (live > peak && !atomic_compare_exchange_weak_explicit(
      &ps5_heap_peak_bytes, &peak, live, memory_order_relaxed, memory_order_relaxed)) {}
}

/* Weak: host builds of this file have no libkernel. */
int sceKernelDebugOutText(int channel, const char *text) __attribute__((weak));
void __wrap_free(void *address);

static void *ps5_heap_record_allocation(void *address, size_t size) {
  const int nonzero = size != 0;
  if (address) {
    ps5_heap_resize_stats(0, sceLibcMspaceMallocUsableSize(address));
    atomic_fetch_add_explicit(&ps5_heap_blocks, 1, memory_order_relaxed);
  } else if (nonzero) {
    if (atomic_fetch_add_explicit(&ps5_heap_failures, 1, memory_order_relaxed) < 8 &&
        sceKernelDebugOutText) {
      char line[128];
      snprintf(line, sizeof(line),
               "[ps5-opengl-heap] mspace refused %zu bytes (live %zu); using the libc heap\n",
               size, atomic_load_explicit(&ps5_heap_live_bytes, memory_order_relaxed));
      sceKernelDebugOutText(0, line);
    }
  }
  return address;
}

static int ps5_heap_ready(void) {
  int state = atomic_load_explicit(&ps5_heap_state, memory_order_acquire);
  if (state == 2)
    return 1;
  if (state != 0)
    return 0;

  int expected = 0;
  if (!atomic_compare_exchange_strong_explicit(
          &ps5_heap_state, &expected, 1, memory_order_acq_rel,
          memory_order_acquire))
    return expected == 2;

  void *base = MAP_FAILED;
  /* The flexible memory of the title is the pool the application allocates
   * from: the pictures and the frames of the video live there. It is the
   * first place to look, and the direct memory - the pool of the graphics
   * part of the console - is the fallback. */
  for (size_t size = ps5_opengl_heap_size; base == MAP_FAILED && size >= (128u << 20);
       size /= 2) {
    base = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (base != MAP_FAILED)
      ps5_heap_size = size;
  }
  if (base == MAP_FAILED) {
    /* CPU-cached direct memory (the driver's own memory type), mapped CPU-only. */
    int64_t physical = 0;
    void *mapped = NULL;
    const size_t alignment = 2u << 20;
    const size_t size = (ps5_opengl_heap_size + alignment - 1) & ~(alignment - 1);
    if (sceKernelAllocateDirectMemory(0, sceKernelGetDirectMemorySize(), size, alignment,
                                      12, &physical) == 0 &&
        sceKernelMapDirectMemory(&mapped, size, PROT_READ | PROT_WRITE, 0, physical,
                                 alignment) == 0 && mapped) {
      base = mapped;
      ps5_heap_size = size;
    }
  }
  if (base == MAP_FAILED) {
    atomic_store_explicit(&ps5_heap_state, -1, memory_order_release);
    return 0;
  }

  ps5_heap_base = base;
  ps5_heap_mspace =
      sceLibcMspaceCreate("PS5-OpenGL", base, PS5_OPENGL_HEAP_SIZE, 0);
  if (ps5_heap_mspace == NULL) {
    ps5_heap_base = NULL;
    munmap(base, PS5_OPENGL_HEAP_SIZE);
    atomic_store_explicit(&ps5_heap_state, -1, memory_order_release);
    return 0;
  }

  atomic_store_explicit(&ps5_heap_state, 2, memory_order_release);
  return 1;
}

size_t __wrap_malloc_usable_size(const void *address);

/* Zero from offset to the end of the block, slack beyond the request included. */
static void *ps5_heap_zero(void *address, size_t offset, size_t size) {
  if (ps5_opengl_heap_zero_fill && address) {
    const size_t usable = __wrap_malloc_usable_size(address);
    if (usable > size)
      size = usable;
    if (size > offset)
      memset((char *)address + offset, 0, size - offset);
  }
  return address;
}

static int ps5_heap_owns(const void *address) {
  /* Acquire publication before reading non-atomic heap metadata. */
  if (atomic_load_explicit(&ps5_heap_state, memory_order_acquire) != 2)
    return 0;
  uintptr_t value = (uintptr_t)address;
  uintptr_t base = (uintptr_t)ps5_heap_base;
  return value >= base && value - base < PS5_OPENGL_HEAP_SIZE;
}

/* When the owned heap is exhausted, allocations continue from the libc heap;
 * free/realloc route by address, so both heaps can hold live blocks. */
void *__wrap_malloc(size_t size) {
  void *result = ps5_heap_ready()
      ? ps5_heap_record_allocation(sceLibcMspaceMalloc(ps5_heap_mspace, size), size)
      : NULL;
  return ps5_heap_zero(result ? result : __real_malloc(size), 0, size);
}

void *__wrap_calloc(size_t count, size_t size) {
  void *result = ps5_heap_ready()
      ? ps5_heap_record_allocation(sceLibcMspaceCalloc(ps5_heap_mspace, count, size),
                                   count * size)
      : NULL;
  return result ? result : __real_calloc(count, size);
}

void *__wrap_realloc(void *address, size_t size) {
  if (address == NULL)
    return __wrap_malloc(size);
  if (!ps5_heap_owns(address)) {
    const size_t before = ps5_opengl_heap_zero_fill ? __real_malloc_usable_size(address) : 0;
    return ps5_heap_zero(__real_realloc(address, size), before, size);
  }
  size_t before = sceLibcMspaceMallocUsableSize(address);
  void *result = sceLibcMspaceRealloc(ps5_heap_mspace, address, size);
  if (result) {
    ps5_heap_resize_stats(before, sceLibcMspaceMallocUsableSize(result));
    ps5_heap_zero(result, before, size);
  } else if (size) {
    atomic_fetch_add_explicit(&ps5_heap_failures, 1, memory_order_relaxed);
    /* Move the block to the libc heap; the original stays valid on failure. */
    result = __real_malloc(size);
    if (result) {
      memcpy(result, address, before < size ? before : size);
      ps5_heap_zero(result, before, size);
      __wrap_free(address);
    }
  } else
    /* Preserve platform realloc(p,0) behavior; NULL does not tell us whether
     * p was freed. Mark the counters inconclusive instead of guessing. */
    atomic_fetch_add_explicit(&ps5_heap_ambiguous_zero_reallocs, 1, memory_order_relaxed);
  return result;
}

void __wrap_free(void *address) {
  if (ps5_heap_owns(address)) {
    ps5_heap_resize_stats(sceLibcMspaceMallocUsableSize(address), 0);
    atomic_fetch_sub_explicit(&ps5_heap_blocks, 1, memory_order_relaxed);
    sceLibcMspaceFree(ps5_heap_mspace, address);
  } else
    __real_free(address);
}

int __wrap_posix_memalign(void **address, size_t alignment, size_t size) {
  int result;
  if (!ps5_heap_ready())
    result = __real_posix_memalign(address, alignment, size);
  else {
    result = sceLibcMspacePosixMemalign(ps5_heap_mspace, address, alignment, size);
    if (result == 0)
      ps5_heap_record_allocation(*address, size);
    else if (result == ENOMEM) {
      atomic_fetch_add_explicit(&ps5_heap_failures, 1, memory_order_relaxed);
      result = __real_posix_memalign(address, alignment, size);
    }
  }
  if (result == 0)
    ps5_heap_zero(*address, 0, size);
  return result;
}

size_t __wrap_malloc_usable_size(const void *address) {
  return ps5_heap_owns(address) ? sceLibcMspaceMallocUsableSize(address)
                                : __real_malloc_usable_size(address);
}

/* Owned-heap usable bytes, for leak diagnostics in long-running titles. */
size_t ps5_opengl_heap_live_bytes(void) {
  return atomic_load_explicit(&ps5_heap_live_bytes, memory_order_relaxed);
}

/* The peak, and how many allocations the heap refused. */
size_t ps5_opengl_heap_peak_bytes(void) {
  return atomic_load_explicit(&ps5_heap_peak_bytes, memory_order_relaxed);
}

size_t ps5_opengl_heap_failures(void) {
  return atomic_load_explicit(&ps5_heap_failures, memory_order_relaxed);
}

/* How much memory the heap holds, so a title can report it. */
size_t ps5_opengl_heap_capacity(void) {
  return (size_t)PS5_OPENGL_HEAP_SIZE;
}

void ps5_opengl_heap_stats_print(unsigned iteration) {
  if (iteration == 0) {
    int state = atomic_load_explicit(&ps5_heap_state, memory_order_acquire);
    printf("[ps5-opengl-cts] mspace state=%d base=%p size=%zu\n",
           state, state == 2 ? ps5_heap_base : NULL, (size_t)PS5_OPENGL_HEAP_SIZE);
  }
}

/* The native import converter rejects unresolved weak application symbols.
 * A diagnostic build's strong definition overrides this default no-op. */
__attribute__((weak)) void ps5_opengl_gpu_snapshot(const char *phase, unsigned iteration) {
  (void)phase;
  (void)iteration;
}
void ps5_opengl_heap_snapshot(const char *phase, unsigned iteration) {
  ps5_opengl_gpu_snapshot(phase, iteration);
  printf("[ps5-opengl-heap] phase=%s sample=%u state=%d live_bytes=%zu peak_bytes=%zu "
         "blocks=%zu failures=%zu ambiguous_zero_reallocs=%zu\n", phase, iteration,
         atomic_load_explicit(&ps5_heap_state, memory_order_acquire),
         atomic_load_explicit(&ps5_heap_live_bytes, memory_order_relaxed),
         atomic_load_explicit(&ps5_heap_peak_bytes, memory_order_relaxed),
         atomic_load_explicit(&ps5_heap_blocks, memory_order_relaxed),
         atomic_load_explicit(&ps5_heap_failures, memory_order_relaxed),
         atomic_load_explicit(&ps5_heap_ambiguous_zero_reallocs, memory_order_relaxed));
}
