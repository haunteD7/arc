#pragma once

#include <ranges>
#include <chrono>
#include <algorithm>
#include <cstring>

class BWTPack
{
public:
  struct Shift
  {
    size_t shift;
  };
  template <std::random_access_iterator Iterator>
  void pack(Iterator begin, Iterator end, size_t block_size)
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
  }
  const std::vector<uint8_t>& get_result() const { return _result; }
private:
  template <std::random_access_iterator Iterator>
  void pack_block(Iterator begin, Iterator end)
  {
    std::vector<uint8_t> data;
    std::vector<Shift> shifts;

    if(begin == end)
      return;

    data.insert(data.end(), begin, end);
    size_t data_size = data.size();
    data.insert(data.end(), begin, end);

    shifts.resize(data_size);
    for(size_t i = 0; i < data_size; i++)
    {
      shifts[i] = { i };
    }
    /* Lexicographical sort */
    auto cmp = [&](const Shift& l, const Shift& r) -> bool
    {
      for (size_t i = 0; i < data_size; i++)
       {
        uint8_t bl = data[i + l.shift];
        uint8_t br = data[i + r.shift];

        if (bl < br) return true;   
        if (bl > br) return false;  
      }
      return false; 
    };
    std::ranges::sort(shifts, cmp);
    size_t og_idx = 0;
    for(size_t i = 0; i < data_size; i++)
    {
      if(shifts[i].shift == 0)
      {
        og_idx = i;
        break;
      }
    }
    for(size_t i = 0; i < data_size; i++)
    {
      _result[i + _pos] = data[shifts[i].shift + data_size - 1];
    }
    std::memcpy(&_result[data_size + _pos], &og_idx, sizeof(size_t));
    _pos += data_size + sizeof(size_t);
  }

  std::vector<uint8_t> _result;
  size_t _pos;
};
