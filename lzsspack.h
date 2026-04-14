#pragma once

#include <vector>
#include <cstdint>
#include <array>
#include <deque>

class LZSSPack
{
public:
  std::vector<uint8_t> pack(const std::vector<uint8_t>& input,
                            size_t window_size = 4096,
                            size_t min_match = 3,
                            size_t max_match = 18)
  {
    std::vector<uint8_t> result;

    if (input.empty())
      return result;

    const uint8_t* data = input.data();
    const size_t size = input.size();

    // hash: 2 bytes -> list of positions (indexes)
    static constexpr size_t HASH_SIZE = 1 << 16;
    std::vector<std::deque<size_t>> table(HASH_SIZE);

    size_t current = 0;

    while (current < size)
    {
      size_t best_length = 0;
      size_t best_offset = 0;

      uint16_t h = 0;
      if (current + 1 < size) // calculate hash
      {
        h = (data[current] << 8) | data[current + 1];
      }

      auto& bucket = table[h];

      int checked = 0;

      for (size_t pos : bucket)
      {
        size_t offset = current - pos;

        if (offset == 0 || offset > window_size)
          continue;

        if (++checked > 32)
          break;

        size_t length = 0;

        while (length < max_match &&
               current + length < size &&
               pos + length < current &&   // out of bounds guard
               data[pos + length] == data[current + length])
        {
          ++length;
        }

        if (length > best_length)
        {
          best_length = length;
          best_offset = offset;

          if (length == max_match)
            break;
        }
      }

      if (best_length >= min_match)
      {
        write_flag(1, result);
        write_uint16(static_cast<uint16_t>(best_offset), result);
        write_uint16(static_cast<uint16_t>(best_length), result);

        // add all positions of a match
        for (size_t i = 0; i < best_length; ++i)
        {
          if (current + i + 1 < size)
          {
            uint16_t hh =
                (data[current + i] << 8) |
                data[current + i + 1];

            auto& b = table[hh];
            b.push_back(current + i);

            // wipe window
            while (!b.empty() &&
                   (current + i) >= b.front() &&
                   (current + i) - b.front() > window_size)
            {
              b.pop_front();
            }
          }
        }

        current += best_length;
      }
      else
      {
        write_flag(0, result);
        result.push_back(data[current]);

        if (current + 1 < size)
        {
          auto& b = table[h];
          b.push_back(current);

          while (!b.empty() &&
                 current >= b.front() &&
                 current - b.front() > window_size)
          {
            b.pop_front();
          }
        }

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
    buffer.push_back(v & 0xFF);
    buffer.push_back((v >> 8) & 0xFF);
  }
};