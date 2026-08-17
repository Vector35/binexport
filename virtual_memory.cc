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

#include "third_party/zynamics/binexport/virtual_memory.h"

#include <cassert>

bool AddressSpace::AddMemoryBlock(Address address, const MemoryBlock& block,
                                  int flags) {
  if (!CanAddMemoryRange(address, block.size())) {
    return false;
  }

  if (!data_.emplace(address, block).second) {
    return false;
  }
  ranges_.emplace(address, block.size());
  flags_.emplace(address, flags);
  return true;
}

bool AddressSpace::AddMemoryRange(Address address, MemoryBlock::size_type size,
                                  int flags) {
  if (!CanAddMemoryRange(address, size)) {
    return false;
  }

  ranges_.emplace(address, size);
  flags_.emplace(address, flags);
  return true;
}

bool AddressSpace::CanAddMemoryRange(Address address,
                                     MemoryBlock::size_type size) const {
  auto it = ranges_.lower_bound(address);
  if (it != ranges_.end() &&
      (it->first == address || it->first - address < size)) {
    return false;
  }
  if (it != ranges_.begin()) {
    --it;
    if (address - it->first < it->second) {
      return false;
    }
  }
  return true;
}

AddressSpace::Ranges::const_iterator AddressSpace::GetMemoryRange(
    Address address) const {
  auto it = ranges_.upper_bound(address);
  if (it != ranges_.begin()) {
    --it;
    if (address - it->first < it->second) {
      return it;
    }
  }
  return ranges_.end();
}

AddressSpace::Data::const_iterator AddressSpace::GetMemoryBlock(
    Address address) const {
  auto it = data_.upper_bound(address);
  if (it != data_.begin()) {
    --it;
    if (address - it->first < it->second.size()) {
      return it;
    }
  }
  return data_.end();
}

AddressSpace::Data::iterator AddressSpace::GetMemoryBlock(Address address) {
  auto it = data_.upper_bound(address);
  if (it != data_.begin()) {
    --it;
    if (address - it->first < it->second.size()) {
      return it;
    }
  }
  return data_.end();
}

const Byte& AddressSpace::operator[](Address address) const {
  const auto memory_block = GetMemoryBlock(address);
  if (memory_block != data_.end()) {
    return memory_block->second[address - memory_block->first];
  }

  assert(IsValidAddress(address));
  const auto sparse_byte = sparse_data_.find(address);
  if (sparse_byte != sparse_data_.end()) {
    return sparse_byte->second;
  }
  static constexpr Byte kZero = 0;
  return kZero;
}

Byte& AddressSpace::operator[](Address address) {
  const auto memory_block = GetMemoryBlock(address);
  if (memory_block != data_.end()) {
    return memory_block->second[address - memory_block->first];
  }

  assert(IsValidAddress(address));
  return sparse_data_[address];
}

bool AddressSpace::IsValidAddress(Address address) const {
  return GetMemoryRange(address) != ranges_.end();
}

bool AddressSpace::IsReadable(Address address) const {
  return GetFlags(address) & kRead;
}

bool AddressSpace::IsWritable(Address address) const {
  return GetFlags(address) & kWrite;
}

bool AddressSpace::IsExecutable(Address address) const {
  return GetFlags(address) & kExecute;
}

int AddressSpace::GetFlags(Address address) const {
  const auto memory_it = GetMemoryRange(address);
  if (memory_it == ranges_.end()) {
    return 0;
  }
  auto flags_it = flags_.find(memory_it->first);
  if (flags_it != flags_.end()) {
    return flags_it->second;
  }
  return 0;
}

// TODO(soerenme) It is somewhat unexpected to have a size method be O(N). Maybe
//     cache the size value in the class?
size_t AddressSpace::size() const {
  size_t value = 0;
  for (const auto& range : ranges_) {
    value += range.second;
  }
  return value;
}
