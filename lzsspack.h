#pragma once

#include <vector>
#include <cstdint>
#include <iterator>
#include <algorithm>

class LZSSPack
{
public:
  template <std::random_access_iterator Iterator>
  std::vector<uint8_t> pack(Iterator begin, Iterator end,
                            size_t window_size = 4096,
                            size_t min_match = 3,
                            size_t max_match = 18)
  {
    std::vector<uint8_t> result;

    Iterator current = begin;

    while (current < end)
    {
      size_t best_length = 0;
      size_t best_offset = 0;

      Iterator window_begin =
          (current - begin > window_size)
              ? current - window_size
              : begin;

      for (Iterator it = window_begin; it < current; ++it)
      {
        // быстрый отсев
        if (*it != *current)
          continue;

        size_t length = 0;

        while (length < max_match &&
               current + length < end &&
               *(it + length) == *(current + length))
        {
          ++length;
        }

        if (length > best_length)
        {
          best_length = length;
          best_offset = current - it;

          // если уже максимум — дальше искать нет смысла
          if (best_length == max_match)
            break;
        }
      }

      if (best_length >= min_match)
      {
        write_flag(1, result);
        write_uint16(static_cast<uint16_t>(best_offset), result);
        write_uint16(static_cast<uint16_t>(best_length), result);

        current += best_length;
      }
      else
      {
        write_flag(0, result);
        result.push_back(static_cast<uint8_t>(*current));
        ++current;
      }
    }

    return result;
  }

private:
  void write_flag(uint8_t f, std::vector<uint8_t>& buffer)
  {
    buffer.push_back(f);
  }

  void write_uint16(uint16_t v, std::vector<uint8_t>& buffer)
  {
    // little-endian
    buffer.push_back(v & 0xFF);
    buffer.push_back((v >> 8) & 0xFF);
  }
};