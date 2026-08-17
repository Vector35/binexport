// Copyright 2011-2024 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VIRTUAL_MEMORY_H_
#define VIRTUAL_MEMORY_H_

#include <atomic>
#include <unordered_map>
#include <vector>

#include "third_party/absl/container/btree_map.h"
#include "third_party/absl/container/flat_hash_map.h"
#include "third_party/zynamics/binexport/util/types.h"

class AddressSpace {
 public:
  enum {
    kRead = 1 << 0,    // Address space is readable.
    kWrite = 1 << 1,   // Address space is writable.
    kExecute = 1 << 2  // Address space is executable.
  };

  using MemoryBlock = std::vector<Byte>;
  using Data = absl::btree_map<Address, MemoryBlock>;
  using Flags = absl::flat_hash_map<Address, int>;

  // Copies the block. Returns true iff the block was added successfully, false
  // if the block overlaps with existing memory.
  bool AddMemoryBlock(Address address, const MemoryBlock& block, int flags);

  // Maps a range without allocating a contiguous backing block. Bytes in the
  // range default to zero and are stored sparsely when written through
  // operator[]. Returns false if the range overlaps with existing memory.
  bool AddMemoryRange(Address address, MemoryBlock::size_type size, int flags);

  // Returns the byte-backed memory block containing address. Range-only
  // mappings are not returned.
  Data::const_iterator GetMemoryBlock(Address address) const;

  Data::iterator GetMemoryBlock(Address address);

  // Returns true iff address is mapped in this address space.
  bool IsValidAddress(Address address) const;

  // Returns true iff the mapping at this address is readable.
  bool IsReadable(Address address) const;

  // Returns true iff the mapping at this address is writable.
  bool IsWritable(Address address) const;

  // Returns true iff the mapping at this address is executable.
  bool IsExecutable(Address address) const;

  // Get flags for a specific address:
  int GetFlags(Address address) const;

  // Size of the entire address space in bytes. Runtime O(number of mapped
  // ranges).
  size_t size() const;

  // Read-only access to byte-backed memory blocks, sorted by ascending address.
  // Range-only mappings are not included.
  const Data& data() const { return data_; }

  // Access the byte at address. Undefined behavior if address is not mapped in
  // this AddressSpace.
  const Byte& operator[](Address address) const;
  Byte& operator[](Address address);

  // Interprets the bytes at address as a little endian value and stores the
  // results. Returns true if the read was successful.
  template <typename T>
  bool ReadLittleEndian(Address address, T* data) const;
  template <typename T>
  bool ReadLittleEndian(const MemoryBlock& memory_block,
                        MemoryBlock::size_type index, T* data) const;

 private:
  using Ranges = absl::btree_map<Address, MemoryBlock::size_type>;
  using SparseData = std::unordered_map<Address, Byte>;

  bool CanAddMemoryRange(Address address, MemoryBlock::size_type size) const;
  Ranges::const_iterator GetMemoryRange(Address address) const;

  Data data_;
  Ranges ranges_;
  SparseData sparse_data_;
  Flags flags_;
};

template <typename T>
bool AddressSpace::ReadLittleEndian(Address address, T* data) const {
  const auto memory_block = GetMemoryBlock(address);
  if (memory_block == data_.end()) {
    return false;
  }
  return ReadLittleEndian(memory_block->second, address - memory_block->first,
                          data);
}

template <typename T>
bool AddressSpace::ReadLittleEndian(const MemoryBlock& memory_block,
                                    MemoryBlock::size_type index,
                                    T* data) const {
  if (!data || index + sizeof(T) > memory_block.size()) {
    return false;
  }
  *data = 0;
  for (T i = 0; i < sizeof(T); ++i) {
    *data |= static_cast<T>(memory_block[index + i]) << (i * 8);
  }
  return true;
}

#endif  // VIRTUAL_MEMORY_H_
