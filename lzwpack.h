#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <iterator>

class LZWPack
{
public:
  template <std::random_access_iterator Iterator>
  void pack(Iterator begin, Iterator end, size_t max_dict_size = 4096)
  {
    _result.clear();

    using Key = std::pair<uint16_t, uint8_t>;

    struct KeyHash
    {
      size_t operator()(const Key &k) const
      {
        return (static_cast<size_t>(k.first) << 8) ^ k.second;
      }
    };

    std::unordered_map<Key, uint16_t, KeyHash> dict;

    uint16_t dict_size = 256;

    // начальное состояние
    uint16_t w = *begin;
    ++begin;

    for (auto it = begin; it != end; ++it)
    {
      uint8_t k = *it;
      Key key = {w, k};

      if (dict.contains(key))
      {
        w = dict[key];
      }
      else
      {
        write_code(w);

        if (dict_size < max_dict_size)
          dict[key] = dict_size++;

        w = k;
      }
    }

    write_code(w);
  }

  const std::vector<uint8_t> &get_result() const { return _result; }

private:
  void write_code(uint16_t code)
  {
    // little-endian
    _result.push_back(code & 0xFF);
    _result.push_back((code >> 8) & 0xFF);
  }

  std::vector<uint8_t> _result;
};