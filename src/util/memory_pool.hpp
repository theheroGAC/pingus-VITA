// src/util/memory_pool.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Pingus - A free Lemmings clone
// Copyright (C) 1999 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef HEADER_PINGUS_UTIL_MEMORY_POOL_HPP
#define HEADER_PINGUS_UTIL_MEMORY_POOL_HPP

#include <assert.h>
#include <stddef.h>
#include <vector>
#include <type_traits>

namespace pingus {


/** MemoryPool allows the allocation of small objects on a previous
    allocated chunk of memory, thus reducing the amount of new/delete
    calls that have to be done and providing a speed up. */
template<class T>
class MemoryPool
{
private:
  typedef std::vector<T*> Objects;
  Objects objects;

  typedef std::vector<char*> Chunks;
  Chunks chunks;

  size_t chunk_size;
  size_t next_free;
  size_t current_chunk_index;

  char* allocate(size_t size)
  {
    assert(size <= chunk_size);
    if (size > chunk_size)
    {
      return nullptr;
    }

    // Check if current chunk has enough space
    if (current_chunk_index < chunks.size() &&
        (next_free + size) <= chunk_size)
    {
      char* ptr = chunks[current_chunk_index] + next_free;
      next_free += size;
      return ptr;
    }

    // Current chunk is full, try to recycle the next chunk
    if (current_chunk_index + 1 < chunks.size())
    {
      current_chunk_index++;
      next_free = 0;
    }
    else
    {
      // Allocate new chunk if needed
      char* chunk = new char[chunk_size];
      chunks.push_back(chunk);
      current_chunk_index = chunks.size() - 1;
      next_free = 0;
    }

    char* ptr = chunks[current_chunk_index] + next_free;
    next_free += size;
    return ptr;
  }

  T* keep(T* t)
  {
    // Only store the pointer if the destructor actually does something
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
      objects.push_back(t);
    }
    return t;
  }

public:
  MemoryPool(size_t chunk_size_ = 16384, size_t initial_chunks = 4) :
    objects(),
    chunks(),
    chunk_size(chunk_size_),
    next_free(0),
    current_chunk_index(0)
  {
    // Pre-allocate chunks to avoid runtime allocation
    chunks.reserve(initial_chunks + 2);
    for (size_t i = 0; i < initial_chunks; ++i)
    {
      chunks.push_back(new char[chunk_size]);
    }

    // Pre-reserve object storage to prevent vector reallocation
    // Only reserve if we are actually going to store objects
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
      size_t estimated_objects = (initial_chunks * chunk_size) / 80;
      objects.reserve(estimated_objects);
    }
  }

  ~MemoryPool()
  {
    // Call destructors on all objects
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
      for(typename Objects::reverse_iterator i = objects.rbegin(); i != objects.rend(); ++i)
        (*i)->~T();
      objects.clear();
    }

    // Free all memory chunks
    for(typename Chunks::reverse_iterator i = chunks.rbegin(); i != chunks.rend(); ++i)
    {
      delete[] *i;
    }
    chunks.clear();
  }

  void clear()
  {
    // Call destructors on all objects
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
      for(typename Objects::reverse_iterator i = objects.rbegin(); i != objects.rend(); ++i)
        (*i)->~T();
      objects.clear();
    }

    // Reset chunk recycling state
    current_chunk_index = 0;
    next_free = 0;
  }

  template<class C>
  T* create() { return keep(new (allocate(sizeof(C))) C()); }

  template<class C, class Arg1>
  T* create(Arg1& arg1) { return keep(new (allocate(sizeof(C))) C(arg1)); }

  template<class C, class Arg1>
  T* create(const Arg1& arg1) { return keep(new (allocate(sizeof(C))) C(arg1)); }

  template<class C, class Arg1, class Arg2>
  T* create(Arg1& arg1, const Arg2& arg2) { return keep(new (allocate(sizeof(C))) C(arg1, arg2)); }

  template<class C, class Arg1, class Arg2>
  T* create(const Arg1& arg1, const Arg2& arg2) { return keep(new (allocate(sizeof(C))) C(arg1, arg2)); }

  template<class C, class Arg1, class Arg2, class Arg3>
  T* create(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3) { return keep(new (allocate(sizeof(C))) C(arg1, arg2, arg3)); }

  template<class C, class Arg1, class Arg2, class Arg3, class Arg4>
  T* create(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4) { return keep(new (allocate(sizeof(C))) C(arg1, arg2, arg3, arg4)); }

  template<class C, class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
  T* create(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5) { return keep(new (allocate(sizeof(C))) C(arg1, arg2, arg3, arg4, arg5)); }

  // Diagnostic functions
  size_t get_allocated_bytes() const { return chunks.size() * chunk_size; }
  size_t get_chunk_count() const { return chunks.size(); }

private:
  MemoryPool (const MemoryPool&);
  MemoryPool& operator= (const MemoryPool&);
};


} // namespace pingus
#endif

// EOF
