//  holdall.c : partie implantation du module holdall.

#include "holdall.h"
#include <stdint.h>
#include <stdio.h>

#define CAPACITY_MIN 1
#define CAPACITY_MUL 5

struct holdall {
  void **aref;
  size_t capacity;
  size_t count;
};

holdall *holdall_empty() {
  holdall *ha = malloc(sizeof *ha);
  if (ha == nullptr) {
    return nullptr;
  }
  ha->aref = malloc(CAPACITY_MIN * sizeof(*ha->aref));
  if (ha->aref == nullptr) {
    free(ha);
    return nullptr;
  }
  ha->capacity = CAPACITY_MIN;
  ha->count = 0;
  return ha;
}

void holdall_dispose(holdall **haptr) {
  if (*haptr == nullptr) {
    return;
  }
  free((*haptr)->aref);
  free(*haptr);
  *haptr = nullptr;
}

int holdall_put(holdall *ha, void *ref) {
  if (ha->count == ha->capacity) {
    if (ha->capacity > SIZE_MAX / CAPACITY_MUL) {
      return -1;
    }
    void **a = realloc(ha->aref, ha->capacity * CAPACITY_MUL * sizeof(*a));
    if (a == nullptr) {
      return -1;
    }
    ha->aref = a;
    ha->capacity *= CAPACITY_MUL;
  }
  ha->aref[ha->count++] = ref;
  return 0;
}

size_t holdall_count(holdall *ha) {
  return ha->count;
}

int holdall_apply(holdall *ha, int (*fun)(void *)) {
  for (size_t i = 0; i < ha->count; i++) {
    int r = fun(ha->aref[i]);
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context(holdall *ha,
    void *context, void *(*fun1)(void *context, void *ptr),
    int (*fun2)(void *ptr, void *resultfun1)) {
  for (size_t i = 0; i < ha->count; i++) {
    int r = fun2(ha->aref[i], fun1(context, ha->aref[i]));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

int holdall_apply_context2(holdall *ha,
    void *context1, void *(*fun1)(void *context1, void *ptr),
    void *context2, int (*fun2)(void *context2, void *ptr, void *resultfun1)) {
  for (size_t i = 0; i < ha->count; i++) {
    int r = fun2(context2, ha->aref[i], fun1(context1, ha->aref[i]));
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

#if defined HOLDALL_EXT && defined WANT_HOLDALL_EXT

void holdall_sort(holdall *ha, int (*compar)(const void *, const void *)) {
  if (ha->count < 2) {
    return;
  }
  qsort(ha->aref, ha->count, sizeof(void *), compar);
}

#endif
