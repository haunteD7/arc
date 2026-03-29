#pragma once

#include "bitreader.h"
#include "bitwriter.h"

class MTFPack
{
public:
  std::vector<uint8_t> pack(std::vector<uint8_t> data, uint8_t symbol_len_bits)
  {
    if(data.empty())
      return {};
    _br.set_data(data);
    
    /* Build dictionary */
    std::vector<uint64_t> dict(1 << symbol_len_bits);
    for(size_t i = 0; i < dict.size(); i++)
      dict[i] = i;

    while(!_br.eof())
    {
      uint64_t sym = _br.get_bits<uint64_t>(symbol_len_bits);
      /* Find symbol in dictionary */
      size_t idx = 0;
      while(sym != dict[idx])
        idx++;
      
      _bw.put_bits(symbol_len_bits, idx);

      /* Move to front */
      for(size_t i = idx; i > 0; i--)
        dict[i] = dict[i - 1];
      dict[0] = sym;
    }
  
    return _bw.take_data();
  }
private:
  BitReader _br;
  BitWriter _bw;
};
