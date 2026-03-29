#pragma once

#include <vector>
#include <cstring>
#include <thread>
#include <cstdint>
#include <future>
#include <algorithm>
#include <iterator>
#include <immintrin.h>

class BWTPack
{
public:
  template <std::random_access_iterator Iterator>
  std::vector<uint8_t> pack(Iterator begin, Iterator end, size_t block_size)
  {
    _result.clear();
    size_t data_size = std::distance(begin, end);
    size_t full_blocks = data_size / block_size;
    size_t last_block_size = data_size % block_size;

    _result.resize(full_blocks * (block_size + sizeof(size_t)) +
                   last_block_size + 2 * sizeof(size_t));

    std::memcpy(_result.data(), &block_size, sizeof(size_t));
    _pos = sizeof(size_t);

    Iterator current = begin;

    // ===================
    // FUTURES для параллельной обработки
    // ===================
    std::vector<std::future<std::pair<std::vector<uint8_t>, size_t>>> futures;

    for (size_t i = 0; i < full_blocks; i++)
    {
      Iterator next = std::next(current, block_size);

      futures.push_back(std::async(std::launch::async, [current, next, this]() {
        return process_block(current, next);
      }));

      current = next;
    }

    // Последний блок
    futures.push_back(std::async(std::launch::async, [current, end, this]() {
      return process_block(current, end);
    }));

    // Собираем результаты
    for (auto& f : futures)
    {
      auto [buf, og_idx] = f.get();
      std::memcpy(_result.data() + _pos, buf.data(), buf.size());
      std::memcpy(_result.data() + _pos + buf.size(), &og_idx, sizeof(size_t));
      _pos += buf.size() + sizeof(size_t);
    }

    return _result;
  }

private:
  // ===========================
  // Построение SA-IS (упрощённое)
  // ===========================
  static std::vector<size_t> build_sa_is(const std::vector<uint8_t>& s)
  {
    size_t n = s.size();
    std::vector<size_t> sa(n);
    for (size_t i = 0; i < n; i++) sa[i] = i;
    // простой линейный radix sort по 1-му байту
    std::array<std::vector<size_t>, 256> buckets{};
    for (size_t i = 0; i < n; i++) buckets[s[i]].push_back(i);
    size_t idx = 0;
    for (int b = 0; b < 256; b++)
      for (size_t v : buckets[b])
        sa[idx++] = v;
    return sa;
  }

  // ===========================
  // SIMD копирование BWT
  // ===========================
  static void bwt_simd_copy(const uint8_t* src, size_t n, std::vector<uint8_t>& dst)
  {
    dst.resize(n);
    size_t i = 0;
    for (; i + 32 <= n; i += 32)
    {
      __m256i v = _mm256_loadu_si256((__m256i*)(src + i));
      _mm256_storeu_si256((__m256i*)(dst.data() + i), v);
    }
    for (; i < n; i++)
      dst[i] = src[i];
  }

  template <std::random_access_iterator Iterator>
  std::pair<std::vector<uint8_t>, size_t> process_block(Iterator begin, Iterator end)
  {
    size_t n = std::distance(begin, end);
    std::vector<uint8_t> data(n * 2);
    std::copy(begin, end, data.begin());
    std::copy(begin, end, data.begin() + n);

    std::vector<uint8_t> view(data.begin(), data.begin() + n);
    auto sa = build_sa_is(view);

    std::vector<uint8_t> buf(n);
    size_t og_idx = 0;
    const uint8_t* d = data.data();

    for (size_t i = 0; i < n; i++)
    {
      size_t j = sa[i];
      if (j == 0) og_idx = i;
      buf[i] = d[j + n - 1];
    }

    return {buf, og_idx};
  }

  std::vector<uint8_t> _result;
  size_t _pos = 0;
};