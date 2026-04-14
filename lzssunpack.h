#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>

class LZSSUnpack
{
public:
  std::vector<uint8_t> unpack(const std::vector<uint8_t>& input)
  {
    std::vector<uint8_t> result;

    const uint8_t* data = input.data();
    size_t size = input.size();

    size_t current = 0;

    while (current < size)
    {
      uint8_t flag = data[current++];

      // literal
      if (flag == 0)
      {
        if (current >= size)
          throw std::runtime_error("Unexpected end of data");

        result.push_back(data[current++]);
      }
      // match
      else
      {
        if (current + 4 > size)
          throw std::runtime_error("Bad compressed data");

        uint16_t offset = read_uint16(data, current, size);
        uint16_t length = read_uint16(data, current, size);

        if (offset == 0 || offset > result.size())
          throw std::runtime_error("Invalid offset");

        size_t start = result.size() - offset;

        for (size_t i = 0; i < length; ++i)
        {
          result.push_back(result[start + i]);
        }
      }
    }

    return result;
  }

private:
  uint16_t read_uint16(const uint8_t* data, size_t& pos, size_t size)
  {
    if (pos + 2 > size)
      throw std::runtime_error("Unexpected end while reading uint16");

    uint16_t v = data[pos];
    v |= static_cast<uint16_t>(data[pos + 1]) << 8;

    pos += 2;
    return v;
  }
};