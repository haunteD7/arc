#pragma once

#include <cstdint>

using OgSymbol = uint64_t; /* Original symbol */
using EcSymbol = uint64_t; /* Encoded symbol */

struct HNode
{
  HNode* left = nullptr;
  HNode* right = nullptr;
  double probability;
  OgSymbol symbol;
  ~HNode()
  {
    if(left) delete left;
    if(right) delete right;
  }
};
struct OgSymbolInfo
{
  OgSymbol code;
  uint8_t ec_len;
};
struct EcSymbolInfo
{
  EcSymbol code;
  uint8_t len;
};

static constexpr uint8_t header_len_len_bits = 64;
static constexpr uint8_t data_len_len_bits = 64;
static constexpr uint8_t encoded_symbol_len_len = 8;