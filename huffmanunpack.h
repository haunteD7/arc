#include "bitreader.h"
#include "bitwriter.h"
#include "huffman.h"

#include <memory>
#include <ranges>
#include <algorithm>
#include <iostream>
#include <set>
#include <bitset>
#include <cmath>
#include <queue>
#include <unordered_map>

class HuffmanUnpack
{
  public:
  std::vector<uint8_t> unpack(std::vector<uint8_t> data, uint8_t symbol_len)
  {
    if(data.empty())
      return {};
    
    _br.set_data(std::move(data));
    /* Read header */
    size_t header_len = _br.get_bits<size_t>(header_len_len_bits);
    size_t data_len = _br.get_bits<size_t>(data_len_len_bits);

    /* Canonical huffman codes */

    std::vector<OgSymbolInfo> symbols(header_len / (symbol_len + encoded_symbol_len_len));

    /* Build array of symbols and it's encoded length */
    while(_br.get_pos() < header_len)
    {
      OgSymbol sym = _br.get_bits<OgSymbol>(symbol_len);
      uint8_t len = _br.get_bits<uint8_t>(encoded_symbol_len_len);
      symbols.push_back({ sym, len });
    }
    /* Sort */
    std::ranges::sort(symbols, [](const OgSymbolInfo& l, const OgSymbolInfo& r) {
      if(l.ec_len == r.ec_len)
        return l.code < r.code;
        
      return l.ec_len < r.ec_len;
    });
    /* Build tree */
    auto root = std::make_unique<HNode>();
    uint8_t prev_len = 0;
    uint64_t code = 0;
    for(auto sym : symbols)
    {
      code <<= (sym.ec_len - prev_len);

      HNode* current = root.get();
      for(int i = sym.ec_len - 1; i >= 0; i--)
      {
        bool bit = (code >> i) & 1ULL;
        if(bit)
        {
          if(current->right == nullptr)
            current->right = new HNode;
          current = current->right;
        }
        else
        {
          if(current->left == nullptr)
            current->left = new HNode;
          current = current->left;
        }
      }
      current->symbol = sym.code;

      code++;
      prev_len = sym.ec_len;
    }
      
    /* Decode data */
    while(_br.get_pos() < data_len)
    {
      HNode* current = root.get();
      while(!(current->left == nullptr && current->right == nullptr))
      {
        bool bit = _br.get();
        if(bit)
          current = current->right;
        else
          current = current->left;
      }
      _bw.put_bits(symbol_len, current->symbol);
      current = root.get();
    }

    return _bw.take_data();
  }
private:
  BitReader _br;
  BitWriter _bw;
};
