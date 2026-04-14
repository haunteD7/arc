#pragma once

#include <vector>
#include <cstdint>
#include <iterator>
#include <stdexcept>

class RLEUnpack
{
public:
  template <std::random_access_iterator Iterator>
  std::vector<uint8_t> unpack(Iterator begin, Iterator end)
  {
    _result.clear();
    
    Iterator current = begin;
    uint8_t repeats_len_bytes = *current; ++current;
    uint8_t symbol_len_bytes = *current; ++current;

    size_t data_size = std::distance(current, end);
    _result.reserve(data_size * 2);

    while (current < end)
    {
      size_t remaining = std::distance(current, end);

      if (remaining < repeats_len_bytes)
        throw std::out_of_range("Not enough data for repeat count");

      // read repeats or literal marker
      size_t repeats_num = read_integer(current, repeats_len_bytes);

      if (repeats_num == 0) // literal
      {
        if (std::distance(current, end) < repeats_len_bytes)
          throw std::out_of_range("Not enough data for literal length");

        size_t literal_len = read_integer(current, repeats_len_bytes);
        size_t literal_bytes = literal_len * symbol_len_bytes;

        if (std::distance(current, end) < literal_bytes)
          throw std::out_of_range("Not enough data for literal symbols");

        _result.insert(_result.end(), current, current + literal_bytes);
        current += literal_bytes;
      }
      else // repeating symbol
      {
        if (std::distance(current, end) < symbol_len_bytes)
          throw std::out_of_range("Not enough data for repeated symbol");

        std::vector<uint8_t> sym(current, current + symbol_len_bytes);
        current += symbol_len_bytes;

        for (size_t i = 0; i < repeats_num; i++)
          _result.insert(_result.end(), sym.begin(), sym.end());
      }
    }

    return std::move(_result);
  }
private:
  // read little endian integer and move iterator
  template <std::random_access_iterator Iterator>
  size_t read_integer(Iterator &it, size_t bytes)
  {
    size_t value = 0;
    for (size_t i = 0; i < bytes; i++)
    {
      value |= static_cast<size_t>(*it) << (8 * i);
      ++it;
    }
    return value;
  }

  std::vector<uint8_t> _result;
};
