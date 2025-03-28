
/* MIT License
*
* Copyright (c) 2021 Ben Hoyt
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

/* NOTE:
 * I just ripped this from Ben Hoyt (https://github.com/benhoyt/ht)
 * I'll probably write my own hashmap at some point but this works for now.
 */

#include "phashmap.h"
#include "defines.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
  const char *key;
  void *value;
} hashmapEntry;

struct hashmap {
  hashmapEntry *entries;
  size_t capacity;
  size_t length;
};

#define INITIAL_CAPACITY 16

hashmap *hashmapNew(void) {
  hashmap *map = (hashmap *)malloc(sizeof(hashmap));

  if (!map) return NULL;

  map->length = 0;
  map->capacity = INITIAL_CAPACITY;

  map->entries = calloc(map->capacity, sizeof(hashmapEntry));
  if (!map->entries) {
    free(map);
    return NULL;
  }

  return map;
}

void hashmapFree(hashmap *m) {
  for (size_t i = 0; i < m->capacity; i++) {
    free((void*)m->entries[i].key);
    free((void*)m->entries[i].value);
  }

  free(m->entries);
  free(m);
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static u64 hash_key(const char* key) {
    u64 hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (u64)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

void *hashmapGet(hashmap *m, const char *key) {
  u64 hash = hash_key(key);
  size_t index = (size_t)(hash & ((u64)m->capacity - 1));
  
  while (m->entries[index].key != NULL) {
    if (strcmp(key, m->entries[index].key) == 0) {
      return m->entries[index].value;
    }

    index++;
    if (index >= m->capacity) {
      index = 0;
    }
  }
  return NULL;
}

// NOTE: what is the point of returning the key if we already have it?
static const char* hashmapSetEntry(hashmapEntry* entries, size_t capacity,
        const char* key, void* value, size_t* plength) {

    u64 hash = hash_key(key);
    size_t index = (size_t)(hash & (u64)(capacity - 1));

    while (entries[index].key != NULL) {
        if (strcmp(key, entries[index].key) == 0) {
            entries[index].value = value;
            return entries[index].key;
        }
        index++;
        if (index >= capacity) {
            index = 0;
        }
    }

    if (plength != NULL) {
        size_t key_len = strlen(key) + 1;
        char *key_copy = (char *)malloc(key_len);
        if (!key_copy) return NULL;
        strcpy(key_copy, key);
        (*plength)++;
        key = key_copy;
    }
    entries[index].key = (char*)key;
    entries[index].value = value;
    return key;
}

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
static bool ht_expand(hashmap* m) {
    // Allocate new entries array.
    size_t new_capacity = m->capacity * 2;
    if (new_capacity < m->capacity) {
        return false;  // overflow (capacity would be too big)
    }
    hashmapEntry* new_entries = calloc(new_capacity, sizeof(hashmapEntry));
    if (new_entries == NULL) {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < m->capacity; i++) {
        hashmapEntry entry = m->entries[i];
        if (entry.key != NULL) {
            hashmapSetEntry(new_entries, new_capacity, entry.key,
                         entry.value, NULL);
        }
    }

    // Free old entries array and update this table's details.
    free(m->entries);
    m->entries = new_entries;
    m->capacity = new_capacity;
    return true;
}

const char* hashmapPush(hashmap* m, const char* key, void* value) {
    assert(value != NULL);
    if (value == NULL) {
        return NULL;
    }

    // If length will exceed half of current capacity, expand it.
    if (m->length >= m->capacity / 2) {
        if (!ht_expand(m)) {
            return NULL;
        }
    }

    // Set entry and update length.
    return hashmapSetEntry(m->entries, m->capacity, key, value,
                        &m->length);
}

// NOTE: if we are obscuring the struct members anyway, why don't we
// make the hashmap a fat pointer similar to the strings?
size_t hashmapLength(hashmap *m) {
  return  m->length;
}

bool hashmapContains(hashmap *m, const char *key) {
    size_t size = hashmapLength(m);
    if (size == 0) return false;

    size_t low = 0;
    size_t high = size;
    while (low < high) {
        size_t mid = (low + high) / 2;
        int c = strcmp(m->entries[mid].key, key);
        if (c == 0) {
            return true;
        }
        if (c < 0) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return false;
}
