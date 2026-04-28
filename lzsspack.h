#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

class LZSSPack
{
public:
  std::vector<uint8_t> pack(const std::vector<uint8_t> &input,
                            size_t window_size = 4096,
                            size_t max_length = 18,
                            size_t min_match = 3)
  {
    std::vector<uint8_t> output;

    size_t i = 0;

    while (i < input.size())
    {
      uint8_t flags = 0;
      size_t flag_pos = output.size();
      output.push_back(0); // placeholder for flags

      for (int bit = 0; bit < 8 && i < input.size(); bit++)
      {
        size_t best_length = 0;
        size_t best_offset = 0;

        size_t start = (i > window_size) ? (i - window_size) : 0;

        for (size_t j = start; j < i; j++)
        {
          size_t length = 0;

          while (length < max_length &&
                 i + length < input.size() &&
                 input[j + length] == input[i + length])
          {
            length++;
          }

          if (length > best_length)
          {
            best_length = length;
            best_offset = i - j;
          }
        }

        if (best_length >= min_match)
        {
          // set bit
          flags |= (1 << bit);

          uint16_t packed =
              ((best_offset & 0xFFF) << 4) |
              ((best_length - min_match) & 0xF);

          output.push_back((packed >> 8) & 0xFF);
          output.push_back(packed & 0xFF);

          i += best_length;
        }
        else
        {
          output.push_back(input[i]);
          i++;
        }
      }

      output[flag_pos] = flags;
    }

    return output;
  }
};