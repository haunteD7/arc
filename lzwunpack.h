#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>

class LZWUnpack
{
public:
  std::vector<uint8_t> unpack(const std::vector<uint8_t>& input,
                              size_t max_dict_size = 4096)
  {
    std::vector<uint8_t> result;

    if (input.size() < 2)
      return result;

    std::vector<std::vector<uint8_t>> dict(256);

    for (int i = 0; i < 256; ++i)
      dict[i] = {static_cast<uint8_t>(i)};

    uint16_t dict_size = 256;

    const uint8_t* data = input.data();
    size_t size = input.size();
    size_t pos = 0;

    auto read_code = [&](size_t& p) -> uint16_t
    {
      if (p + 2 > size)
        throw std::runtime_error("Unexpected end");

      uint16_t code = data[p];
      code |= static_cast<uint16_t>(data[p + 1]) << 8;
      p += 2;
      return code;
    };

    uint16_t prev_code = read_code(pos);

    if (prev_code >= dict.size())
      throw std::runtime_error("Bad LZW stream");

    std::vector<uint8_t> prev = dict[prev_code];
    result.insert(result.end(), prev.begin(), prev.end());

    while (pos < size)
    {
      uint16_t code = read_code(pos);

      std::vector<uint8_t> entry;

      if (code < dict_size)
      {
        entry = dict[code];
      }
      else if (code == dict_size)
      {
        entry = prev;
        entry.push_back(prev[0]);
      }
      else
      {
        throw std::runtime_error("Bad LZW code");
      }

      result.insert(result.end(), entry.begin(), entry.end());

      if (dict_size < max_dict_size)
      {
        std::vector<uint8_t> new_entry = prev;
        new_entry.push_back(entry[0]);
        dict.push_back(std::move(new_entry));
        ++dict_size;
      }

      prev = std::move(entry);
    }

    return result;
  }
};