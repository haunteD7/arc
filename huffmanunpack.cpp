#include "bitreader.h"
#include "bitwriter.h"
#include "utils.h"
#include "huffman.h"

#include <memory>
#include <ranges>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <queue>
#include <map>

class HuffmanUnpack
{
public:
  void unpack(const std::vector<uint8_t>& data)
  {
    if(data.empty())
      return;

    BitReader header_br;
    header_br.set_data(data);

    size_t header_len = header_br.get_bits<size_t>(header_len_len);
    uint8_t symbol_len = header_br.get_bits<uint8_t>(symbol_len_len) + 1;
    uint8_t padding = header_br.get_bits<uint8_t>(3);

    /* Canonical huffman codes */

    std::vector<OgSymbolInfo> symbols;
    symbols.reserve(header_len / (symbol_len + encoded_symbol_len_len));

    OgSymbol sym = 0;
    uint8_t prev_len = 0;

    /* Build array of symbols and it's encoded length */
    while(header_br.get_pos() < header_len)
    {
      sym = header_br.get_bits<OgSymbol>(symbol_len);
      uint8_t len = header_br.get_bits<uint8_t>(encoded_symbol_len_len) + 1;
    }
    std::ranges::sort(symbols, [](const OgSymbolInfo& l, const OgSymbolInfo& r) {
      if(l.ec_len == r.ec_len)
        return l.code < r.code;
        
      return l.ec_len < r.ec_len;
    });

    std::map<EcSymbol, OgSymbolInfo> dict;
    EcSymbol code = 0;
    uint8_t prev_len = 0;
    for(auto sym : symbols)
    {
      code <<= (sym.ec_len - prev_len);

      dict[code] = sym;
      
      code++;
      prev_len = sym.ec_len;
    }
      
    _br.set_data(data, padding);
    _br.move_forward(header_br.get_pos());

    while(!_br.eof())
    {
      
    }
  }
  const std::vector<uint8_t>& get_result() { return _bw.get_data(); }
private:
  BitReader _br;
  BitWriter _bw;
};

int main(int argc, char const *argv[])
{
  auto data = read_file_by_args(argc, argv, "a.bin");
  if(!data.has_value())
    return -1;
  
  HuffmanUnpack unpacker;
  unpacker.unpack(data.value());
  
  write_file_by_args(argc, argv, unpacker.get_result(), "a.txt");
  return 0;
}