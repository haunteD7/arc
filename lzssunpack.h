#pragma once

#include <vector>
#include <cstdint>

class LZSSUnpack
{
public:
  std::vector<uint8_t> unpack(const std::vector<uint8_t> &input)
  {
    std::vector<uint8_t> output;

    size_t i = 0;

    while (i < input.size())
    {
      uint8_t flags = input[i++];

      for (int bit = 0; bit < 8 && i < input.size(); bit++)
      {
        if (flags & (1 << bit))
        {
          // match
          uint16_t packed =
              (input[i] << 8) | input[i + 1];
          i += 2;

          size_t offset = (packed >> 4) & 0xFFF;
          size_t length = (packed & 0xF) + 3;

          size_t start = output.size() - offset;

          for (size_t j = 0; j < length; j++)
          {
            output.push_back(output[start + j]);
          }
        }
        else
        {
          // literal
          output.push_back(input[i++]);
        }
      }
    }

    return output;
  }
};