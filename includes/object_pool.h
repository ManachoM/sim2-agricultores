
#ifndef _GENERIC_OBJECT_PARTITIONER_H_
#define _GENERIC_OBJECT_PARTITIONER_H_

#include <cstddef>
#include <cstdio>
#include <iterator>
#include <type_traits>
#include <utility>

template <typename T, std::size_t chunk_size = 1'024> class ObjectPool {
public:
  ObjectPool() : free_slot(nullptr), head_chunk(nullptr) {
    this->allocate_chunk();
  };

  template <typename... Args> T *alloc(Args &&...args) {
    if (!this->free_slot) {
      this->allocate_chunk();
    }
    slot *first_slot = this->free_slot;
    this->free_slot = first_slot->next;

    T *instance = new (&first_slot->storage) T(std::forward<Args>(args)...);
    return instance;
  };

  void release(T *obj) {
    if (!obj)
      return;
    // Get the slot from the object pointer
    slot *slot = reinterpret_cast<union slot *>(reinterpret_cast<char *>(obj));

    // Destroy the object
    obj->~T();

    // Add the slot back to the free list
    slot->next = this->free_slot;
    this->free_slot = slot;

    if (should_release_chunks())
      release_unused_chunks();
  };

  ~ObjectPool() {
    chunk *hchunk = this->head_chunk;

    printf("Borrando ObjectPool...\n");
    while (hchunk) {
      for (std::size_t i = 0; i < chunk_size; ++i) {

        slot *s = &hchunk->slots[i];
        T *obj = reinterpret_cast<T *>(&s->storage);
        obj->~T();
      }
      chunk *next_chunk = hchunk->next;
      delete hchunk;
      hchunk = next_chunk;
    }
  };

private:
  union slot {
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
    slot *next;
  };

  struct chunk {
    chunk *next;
    slot slots[chunk_size];
  };

  slot *free_slot;
  chunk *head_chunk;
  void allocate_chunk() {
    chunk *new_chunk = new chunk();
    new_chunk->next = this->head_chunk;
    this->head_chunk = new_chunk;
    for (std::size_t i = 0; i < chunk_size; ++i) {
      slot *slot = &(new_chunk->slots[i]);
      slot->next = this->free_slot;
      this->free_slot = slot;
    };
    this->free_slot = this->head_chunk->slots;
  };

  bool should_release_chunks() {
    if (num_chunks <= 1)
      return false;

    float utilization = (float)active_objects / (num_chunks * chunk_size);
    return utilization < (1.0f / clean_threashold);
  };

  void release_unused_chunks() {
    // Keep at least one chunk
    if (num_chunks <= 1)
      return;

    // Find chunks that are completely unused
    chunk *prev = nullptr;
    chunk *current = head_chunk;

    while (current && num_chunks > 1) {
      bool chunk_in_use = false;

      // Check if any slot in this chunk is in use
      for (std::size_t i = 0; i < chunk_size; ++i) {
        slot *s = &current->slots[i];
        // If this slot is not in the free list, the chunk is in use
        if (slot_in_use(s)) {
          chunk_in_use = true;
          break;
        }
      }

      if (!chunk_in_use) {
        // This chunk can be deleted
        if (prev) {
          prev->next = current->next;
        } else {
          head_chunk = current->next;
        }

        chunk *to_delete = current;
        current = current->next;
        delete to_delete;
        num_chunks--;
      } else {
        prev = current;
        current = current->next;
      }
    }

    // Rebuild the free list after chunk deletion
    rebuild_free_list();
  }

  bool slot_in_use(slot *s) {
    // Check if this slot is in the free list
    slot *current = free_slot;
    while (current) {
      if (current == s)
        return false; // Found in free list
      current = current->next;
    }
    return true; // Not in free list, must be in use
  }

  void rebuild_free_list() {
    // Clear existing free list
    free_slot = nullptr;

    // Add all unused slots to the free list
    chunk *current = head_chunk;
    while (current) {
      for (std::size_t i = 0; i < chunk_size; ++i) {
        slot *s = &current->slots[i];
        if (!slot_in_use(s)) {
          s->next = free_slot;
          free_slot = s;
        }
      }
      current = current->next;
    }
  };

  std::size_t active_objects = 0;
  std::size_t max_active_objects = 0;
  std::size_t num_chunks = 0;
  std::size_t clean_threashold = 3;
};

#endif // !_GENERIC_OBJECT_PARTITIONER_H_
