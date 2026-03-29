#pragma once

#include <vector>
#include <array>
#include <iterator>
#include <cstring>
#include <cstdint>
#include <algorithm>

class BWTUnpack
{
public:
  template <std::random_access_iterator Iterator>
  std::vector<uint8_t> unpack(Iterator begin, Iterator end)
  {
    _result.clear();

    size_t data_size = std::distance(begin, end);
    if (data_size <= sizeof(size_t))
      return {};

    /* Read block size */
    size_t block_size = 0;
    std::memcpy(&block_size, std::addressof(*begin), sizeof(size_t));

    Iterator current = begin + sizeof(size_t);

    while (current < end)
    {
      /* Determine size of current block */
      size_t remaining = std::distance(current, end);

      if (remaining <= sizeof(size_t))
        break;

      size_t payload_size = std::min(block_size, remaining - sizeof(size_t));

      Iterator block_begin = current;
      Iterator block_end = current + payload_size + sizeof(size_t);

      unpack_block(block_begin, block_end);

      current = block_end;
    }

    return std::move(_result);
  }
private:
  template <std::random_access_iterator Iterator>
  void unpack_block(Iterator begin, Iterator end)
  {
    size_t block_total = std::distance(begin, end);
    if (block_total <= sizeof(size_t))
      return;

    size_t N = block_total - sizeof(size_t);

    /* Read index */
    size_t og_idx = 0;
    std::memcpy(&og_idx, std::addressof(*(begin + N)), sizeof(size_t));

    Iterator L_begin = begin;

    /* Count */
    std::array<size_t, 256> count{};
    for (Iterator it = L_begin; it != L_begin + N; ++it)
      count[*it]++;

    /* Start */
    std::array<size_t, 256> start{};
    size_t sum = 0;
    for (size_t c = 0; c < 256; c++)
    {
      if (count[c])
      {
        start[c] = sum;
        sum += count[c];
      }
    }

    /* LF */
    std::array<size_t, 256> seen{};
    std::vector<size_t> LF(N);

    for (size_t i = 0; i < N; i++)
    {
      uint8_t c = *(L_begin + i);
      LF[i] = start[c] + seen[c];
      seen[c]++;
    }

    /* Decode */
    size_t old_size = _result.size();
    _result.resize(old_size + N);

    size_t idx = og_idx;

    for (size_t i = N; i-- > 0;)
    {
      _result[old_size + i] = *(L_begin + idx);
      idx = LF[idx];
    }
  }

  std::vector<uint8_t> _result;
};