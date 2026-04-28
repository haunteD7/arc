#pragma once

#include "huffmanpack.h"

#include <vector>
#include <array>
#include <cstdint>

class ArithmeticPack
{
public:
  std::vector<uint8_t> pack(const std::vector<uint8_t> &data)
  {
    if (data.empty())
      return {};

    auto prob = calculate_probabilities(data);
    auto cum = build_cumulative(prob);

    double low = 0.0;
    double high = 1.0;

    for (uint8_t symbol : data)
    {
      double range = high - low;

      high = low + range * cum[symbol + 1];
      low = low + range * cum[symbol];
    }

    // Take number in interval's bounds
    double code = (low + high) * 0.5;

    return encode_double(code);
  }

private:
  static std::array<double, 257> build_cumulative(const std::array<double, 256> &prob)
  {
    std::array<double, 257> cum{};
    cum[0] = 0.0;

    for (int i = 0; i < 256; ++i)
    {
      cum[i + 1] = cum[i] + prob[i];
    }

    return cum;
  }

  static std::vector<uint8_t> encode_double(double value)
  {
    std::vector<uint8_t> out(sizeof(double));
    uint8_t *ptr = reinterpret_cast<uint8_t *>(&value);

    for (size_t i = 0; i < sizeof(double); ++i)
    {
      out[i] = ptr[i];
    }

    return out;
  }
};