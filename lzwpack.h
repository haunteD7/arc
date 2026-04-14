#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <utility>

class LZWPack
{
public:
  std::vector<uint8_t> pack(const std::vector<uint8_t>& input,
                            size_t max_dict_size = 4096)
  {
    std::vector<uint8_t> result;

    if (input.empty())
      return result;

    struct Key
    {
      uint16_t w;
      uint8_t k;

      bool operator==(const Key& other) const
      {
        return w == other.w && k == other.k;
      }
    };

    struct KeyHash
    {
      size_t operator()(const Key& key) const
      {
        return (static_cast<size_t>(key.w) << 8) ^ key.k;
      }
    };

    std::unordered_map<Key, uint16_t, KeyHash> dict;
    dict.reserve(1 << 16);

    uint16_t dict_size = 256;

    const uint8_t* data = input.data();
    size_t size = input.size();

    size_t i = 0;

    uint16_t w = data[i++];

    while (i < size)
    {
      uint8_t k = data[i++];
      Key key{w, k};

      auto it = dict.find(key);

      if (it != dict.end())
      {
        w = it->second;
      }
      else
      {
        write_code(w, result);

        if (dict_size < max_dict_size)
        {
          dict[key] = dict_size++;
        }

        w = k;
      }
    }

    write_code(w, result);

    return result;
  }

private:
  void write_code(uint16_t code, std::vector<uint8_t>& buffer)
  {
    buffer.push_back(code & 0xFF);
    buffer.push_back((code >> 8) & 0xFF);
  }
};  