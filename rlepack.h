#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <cstring>

class RLEPack
{
public:
  template <std::random_access_iterator Iterator>
  void pack(Iterator begin, Iterator end, size_t repeats_len_bytes, size_t symbol_len_bytes)
  {
    _result.clear();

    if (begin == end)
      return;

    size_t data_size = std::distance(begin, end);
    if (data_size % symbol_len_bytes != 0)
      throw std::out_of_range("Data size not divisible by symbol length");

    const size_t max_repeats = (1ULL << (repeats_len_bytes * 8)) - 1;

    std::vector<uint8_t> literal;

    // Читаем первый символ
    std::vector<uint8_t> sym(symbol_len_bytes);
    std::copy_n(begin, symbol_len_bytes, sym.begin());
    size_t repeats_num = 1;

    Iterator current = begin + symbol_len_bytes;

    while (current < end)
    {
      // Читаем следующий символ
      std::vector<uint8_t> next_sym(symbol_len_bytes);
      std::copy_n(current, symbol_len_bytes, next_sym.begin());

      if (sym == next_sym && repeats_num < max_repeats)
      {
        repeats_num++;
      }
      else
      {
        if (repeats_num == 1)
        {
          literal.insert(literal.end(), sym.begin(), sym.end());
          if (literal.size() / symbol_len_bytes >= max_repeats)
            flush_literal(literal, symbol_len_bytes, repeats_len_bytes);
        }
        else
        {
          flush_literal(literal, symbol_len_bytes, repeats_len_bytes);
          put_sequence(repeats_num, sym, symbol_len_bytes, repeats_len_bytes);
        }

        repeats_num = 1;
        sym = next_sym;
      }

      current += symbol_len_bytes;
    }

    // Финальная запись
    if (repeats_num == 1)
    {
      literal.insert(literal.end(), sym.begin(), sym.end());
      flush_literal(literal, symbol_len_bytes, repeats_len_bytes);
    }
    else
    {
      flush_literal(literal, symbol_len_bytes, repeats_len_bytes);
      put_sequence(repeats_num, sym, symbol_len_bytes, repeats_len_bytes);
    }
  }

  const std::vector<uint8_t> &get_result() const { return _result; }

private:
  void put_sequence(size_t repeats_num, const std::vector<uint8_t> &sym,
                    size_t symbol_len_bytes, size_t repeats_len_bytes)
  {
    write_integer(repeats_num, repeats_len_bytes);
    _result.insert(_result.end(), sym.begin(), sym.end());
  }

  void flush_literal(std::vector<uint8_t> &literal,
                     size_t symbol_len_bytes, size_t repeats_len_bytes)
  {
    if (literal.empty())
      return;

    write_integer(0, repeats_len_bytes);                                 // литерал-маркер
    write_integer(literal.size() / symbol_len_bytes, repeats_len_bytes); // длина литералов
    _result.insert(_result.end(), literal.begin(), literal.end());
    literal.clear();
  }

  void write_integer(size_t value, size_t bytes)
  {
    for (size_t i = 0; i < bytes; i++)
    {
      _result.push_back(static_cast<uint8_t>(value & 0xFF));
      value >>= 8;
    }
  }

  std::vector<uint8_t> _result;
};