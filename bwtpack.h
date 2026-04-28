#pragma once

#include <ranges>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <vector>
#include <cstdint>

class BWTPack
{
public:
  struct Shift
  {
    size_t shift;
  };
  
  template <std::random_access_iterator Iterator>
  std::vector<uint8_t> pack(Iterator begin, Iterator end, size_t block_size)
  {
    _result.clear();

    size_t data_size = std::distance(begin, end);
    size_t full_blocks = data_size / block_size;
    size_t last_block_size = data_size % block_size;

    _result.resize(full_blocks * (block_size + sizeof(size_t)) + last_block_size + 2 * sizeof(size_t));
    std::memcpy(&_result[0], &block_size, sizeof(size_t));
    _pos = sizeof(size_t);
    Iterator current_begin = begin;
    for(size_t i = 0; i < full_blocks; i++)
    {
      Iterator current_end = std::next(current_begin, block_size);
      pack_block(current_begin, current_end);
      current_begin = current_end;
    }
    pack_block(current_begin, end);

    return std::move(_result);
  }
  
private:
  // uses suffix array
  template <std::random_access_iterator Iterator>
  void pack_block(Iterator begin, Iterator end)
  {
    if(begin == end)
      return;

    size_t n = std::distance(begin, end);
    
    std::vector<uint8_t> data;
    data.reserve(n);
    data.insert(data.end(), begin, end);
    
    // dobule the data
    std::vector<uint8_t> cyclic_data;
    cyclic_data.reserve(2 * n);
    cyclic_data.insert(cyclic_data.end(), data.begin(), data.end());
    cyclic_data.insert(cyclic_data.end(), data.begin(), data.end());
    
    // build suffix array
    std::vector<size_t> suffix_array = build_suffix_array(cyclic_data, n);
    
    // find index of the original string
    size_t original_index = 0;
    for(size_t i = 0; i < n; i++)
    {
      if(suffix_array[i] == 0)
      {
        original_index = i;
        break;
      }
    }
    
    // get last elements of all suffix arrays
    for(size_t i = 0; i < n; i++)
    {
      size_t pos = suffix_array[i];
      _result[_pos + i] = cyclic_data[pos + n - 1];
    }
    
    // save index of original string
    std::memcpy(&_result[n + _pos], &original_index, sizeof(size_t));
    _pos += n + sizeof(size_t);
  }
  
  std::vector<size_t> build_suffix_array(const std::vector<uint8_t>& data, size_t limit)
  {
    size_t n = data.size();
    std::vector<size_t> suffix_array(n);
    std::vector<size_t> rank(n);
    std::vector<size_t> new_rank(n);
    
    // initialize
    for(size_t i = 0; i < n; i++)
    {
      suffix_array[i] = i;
      rank[i] = data[i];
    }
    
    auto cmp = [&](size_t a, size_t b) -> bool
    {
      if(rank[a] != rank[b])
        return rank[a] < rank[b];
      size_t ra = (a + 1 < n) ? rank[a + 1] : 0;
      size_t rb = (b + 1 < n) ? rank[b + 1] : 0;
      return ra < rb;
    };
    
    std::sort(suffix_array.begin(), suffix_array.end(), cmp);
    
    new_rank[suffix_array[0]] = 0;
    for(size_t i = 1; i < n; i++)
    {
      new_rank[suffix_array[i]] = new_rank[suffix_array[i - 1]] + 
        (cmp(suffix_array[i - 1], suffix_array[i]) ? 1 : 0);
    }
    rank.swap(new_rank);
    
    for(size_t k = 2; k < n; k <<= 1)
    {
      auto cmp_k = [&](size_t a, size_t b) -> bool
      {
        if(rank[a] != rank[b])
          return rank[a] < rank[b];
        size_t ra = (a + k/2 < n) ? rank[a + k/2] : 0;
        size_t rb = (b + k/2 < n) ? rank[b + k/2] : 0;
        return ra < rb;
      };
      
      std::sort(suffix_array.begin(), suffix_array.end(), cmp_k);
      
      new_rank[suffix_array[0]] = 0;
      for(size_t i = 1; i < n; i++)
      {
        new_rank[suffix_array[i]] = new_rank[suffix_array[i - 1]] + 
          (cmp_k(suffix_array[i - 1], suffix_array[i]) ? 1 : 0);
      }
      rank.swap(new_rank);
      
      if(rank[suffix_array[n - 1]] == n - 1)
        break;
    }
    
    std::vector<size_t> result;
    result.reserve(limit);
    for(size_t i = 0; i < n && result.size() < limit; i++)
    {
      if(suffix_array[i] < limit)
        result.push_back(suffix_array[i]);
    }
    
    return result;
  }

  std::vector<uint8_t> _result;
  size_t _pos;
};
