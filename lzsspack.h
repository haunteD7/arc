#pragma once

#include <vector>
#include <cstdint>
#include <iterator>
#include <algorithm>

class LZSSPack
{
public:
  template <std::random_access_iterator Iterator>
  void pack(Iterator begin, Iterator end,
            size_t window_size = 4096,
            size_t min_match = 3,
            size_t max_match = 18)
  {
    _result.clear();

    Iterator current = begin;

    while (current < end)
    {
      size_t best_length = 0;
      size_t best_offset = 0;

      // границы окна
      Iterator window_begin = (current - begin > window_size)
                                  ? current - window_size
                                  : begin;

      // ищем лучшее совпадение
      for (Iterator it = window_begin; it < current; ++it)
      {
        size_t length = 0;

        while (length < max_match &&
               current + length < end &&
               *(it + length) == *(current + length))
        {
          length++;
        }

        if (length > best_length)
        {
          best_length = length;
          best_offset = current - it;
        }
      }

      // если нашли нормальное совпадение
      if (best_length >= min_match)
      {
        write_flag(1);
        write_uint16(best_offset);
        write_uint16(best_length);

        current += best_length;
      }
      else
      {
        write_flag(0);
        _result.push_back(*current);
        ++current;
      }
    }
  }

  const std::vector<uint8_t> &get_result() const { return _result; }

private:
  void write_flag(uint8_t f)
  {
    _result.push_back(f);
  }

  void write_uint16(uint16_t v)
  {
    // little-endian
    _result.push_back(v & 0xFF);
    _result.push_back((v >> 8) & 0xFF);
  }

  std::vector<uint8_t> _result;
};