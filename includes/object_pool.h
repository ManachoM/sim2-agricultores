#ifndef _GENERIC_OBJECT_PARTITIONER_H_
#define _GENERIC_OBJECT_PARTITIONER_H_

#include <cstddef>
#include <type_traits>

template <typename T, std::size_t chunk_size = 1'024> class ObjectPool
{
public:
  ObjectPool() : free_slot(nullptr), head_chunk(nullptr)
  {
    this->allocate_chunk();
  };
  template <typename... Args> T *alloc(Args &&...args)
  {
    if (!this->free_slot)
    {
      this->allocate_chunk();
    }

    slot *first_slot = this->free_slot;
    this->free_slot = first_slot->next;

    T *instance = new (&free_slot->storage) T(std::forward<Args>(args)...);
    return instance;
  };
  void release(T *obj)
  {
    if (!obj)
      return;
    // Get the slot from the object pointer
    slot *slot = reinterpret_cast<union slot *>(reinterpret_cast<char *>(obj));

    // Destroy the object
    obj->~T();

    // Add the slot back to the free list
    slot->next = this->free_slot;
    this->free_slot = slot;
  };
  ~ObjectPool()
  {
    chunk *hchunk = this->head_chunk;
    while (hchunk)
    {
      for (std::size_t i = 0; i < chunk_size; ++i)
      {
        slot *s = hchunk->slots[i];
        T *obj = reinterpret_cast<T *>(&s->storage);
        obj->~T();
      }
      chunk *next_chunk = hchunk->next;
      delete hchunk;
      hchunk = next_chunk;
    }
  };

private:
  union slot
  {
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
    slot *next;
  };

  struct chunk
  {
    chunk *next;
    slot slots[chunk_size];
  };

  slot *free_slot;
  chunk *head_chunk;
  void allocate_chunk()
  {
    chunk *new_chunk = new chunk();
    new_chunk->next = this->head_chunk;
    this->head_chunk = new_chunk;
    for (std::size_t i = 0; i < chunk_size; ++i)
    {
      slot *slot = &new_chunk[i].slots[i];
      slot->next = this->free_slot;
      this->free_slot = slot;
    };
  };
};

#endif // !_GENERIC_OBJECT_PARTITIONER_H_
