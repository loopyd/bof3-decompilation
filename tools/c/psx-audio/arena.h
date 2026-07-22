#ifndef PSX_AUDIO_ARENA_H
#define PSX_AUDIO_ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_DEFAULT_ALIGN 16

typedef struct {
  uint8_t* base;
  size_t   capacity;
  size_t   used;
} Arena;

typedef struct {
  size_t used;
} ArenaMark;

static inline int arena_init(Arena* a, size_t capacity) {
  a->base = (uint8_t*)malloc(capacity);
  a->capacity = a->base ? capacity : 0;
  a->used = 0;
  return a->base != NULL;
}

static inline void* arena_alloc(Arena* a, size_t size, size_t align) {
  size_t offset = (a->used + align - 1) & ~(align - 1);
  void*  ptr;

  if (offset + size > a->capacity)
    return NULL;
  ptr = a->base + offset;
  a->used = offset + size;
  return ptr;
}

static inline void* arena_calloc(Arena* a, size_t count, size_t size,
                                 size_t align) {
  void* ptr = arena_alloc(a, count * size, align);
  if (ptr)
    memset(ptr, 0, count * size);
  return ptr;
}

static inline ArenaMark arena_save(const Arena* a) {
  ArenaMark m;
  m.used = a->used;
  return m;
}

static inline void arena_restore(Arena* a, ArenaMark mark) {
  a->used = mark.used;
}

static inline void arena_reset(Arena* a) {
  a->used = 0;
}

static inline void arena_destroy(Arena* a) {
  free(a->base);
  a->base = NULL;
  a->capacity = 0;
  a->used = 0;
}

static inline size_t arena_remaining(const Arena* a) {
  return a->capacity - a->used;
}

#endif
