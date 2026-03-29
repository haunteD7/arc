#pragma once

#include <vector>
#include <cstdint>
#include <iterator>
#include <stdexcept>

class LZSSUnpack
{
public:
  template <std::random_access_iterator Iterator>
  std::vector<uint8_t> unpack(Iterator begin, Iterator end)
  {
    _result.clear();

    Iterator current = begin;

    while (current < end)
    {
      uint8_t flag = *current++;

      if (flag == 0)
      {
        if (current >= end)
          throw std::runtime_error("Unexpected end");

        _result.push_back(*current++);
      }
      else
      {
        if (std::distance(current, end) < 4)
          throw std::runtime_error("Bad compressed data");

        uint16_t offset = read_uint16(current);
        uint16_t length = read_uint16(current);

        size_t start = _result.size() - offset;

        for (size_t i = 0; i < length; i++)
        {
          _result.push_back(_result[start + i]);
        }
      }
    }

    return std::move(_result);
  }
private:
  template <typename Iterator>
  uint16_t read_uint16(Iterator &it)
  {
    uint16_t v = *it;
    ++it;
    v |= static_cast<uint16_t>(*it) << 8;
    ++it;
    return v;
  }

  std::vector<uint8_t> _result;
};