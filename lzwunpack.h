#pragma once

#include <vector>
#include <cstdint>
#include <iterator>

class LZWUnpack
{
public:
  template <std::random_access_iterator Iterator>
  void unpack(Iterator begin, Iterator end, size_t max_dict_size = 4096)
  {
    _result.clear();

    if (begin == end)
      return;

    std::vector<std::vector<uint8_t>> dict(256);

    for (int i = 0; i < 256; i++)
      dict[i] = {static_cast<uint8_t>(i)};

    uint16_t dict_size = 256;

    uint16_t prev_code = read_code(begin);
    auto prev = dict[prev_code];

    _result.insert(_result.end(), prev.begin(), prev.end());

    while (begin < end)
    {
      uint16_t code = read_code(begin);

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

      _result.insert(_result.end(), entry.begin(), entry.end());

      if (dict_size < max_dict_size)
      {
        auto new_entry = prev;
        new_entry.push_back(entry[0]);
        dict.push_back(std::move(new_entry));
        dict_size++;
      }

      prev = std::move(entry);
    }
  }

  const std::vector<uint8_t> &get_result() const { return _result; }

private:
  template <typename Iterator>
  uint16_t read_code(Iterator &it)
  {
    uint16_t code = *it;
    ++it;
    code |= static_cast<uint16_t>(*it) << 8;
    ++it;
    return code;
  }

  std::vector<uint8_t> _result;
};