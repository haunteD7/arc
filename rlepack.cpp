#include "utils.h"
#include "bitreader.h"
#include "bitwriter.h"

class RLEPack
{
public:
  void pack(const std::vector<uint8_t>& data, uint8_t repeats_len, uint8_t symbol_len)
  {
    const size_t max_repeats = (1ULL << repeats_len) - 1; /* Max value that can be written in *repeats_len* bytes */

    _repeats_len = repeats_len;
    _symbol_len = symbol_len;

    if(data.empty())
      return;
    
    _br.set_data(data);
    _bw.put_bits(5, repeats_len - 1);
    _bw.put_bits(5, symbol_len - 1);
    size_t padding_pos = _bw.get_pos();
    _bw.put_bits(3, 0); // Padding
    
    uint64_t sym = _br.get_bits<uint64_t>(symbol_len);
    size_t repeats_num = 1;
    while(!_br.eof())
    {
      uint64_t next_sym = _br.get_bits<uint64_t>(symbol_len);

      if(sym == next_sym && repeats_num != max_repeats)
      {
        repeats_num++;
      }
      else
      {
        if(repeats_num == 1) /* If the symbol sym repeated only 1 time */
        {
          _literal.push_back(sym);

          if(_literal.size() == max_repeats)
            flush_literal();
        }
        else
        {
          flush_literal();
          put_sequence(repeats_num, sym);
        }
        repeats_num = 1;
        sym = next_sym;
      }
    }
    if(repeats_num == 1) /* If the symbol sym repeated only 1 time */
    {
      _literal.push_back(sym);
      flush_literal();
    }
    else
    {
      flush_literal();
      put_sequence(repeats_num, sym);
    }

    _bw.set_pos(padding_pos);
    _bw.put_bits(8, _bw.len() % 8); /* Put padding */
  }
  const std::vector<uint8_t>& get_result() { return _bw.get_data(); }
private:
  void put_sequence(size_t repeats_num, uint64_t sym)
  {
    _bw.put_bits(_repeats_len, repeats_num);
    _bw.put_bits(_symbol_len, sym);
  }
  void flush_literal()
  {
    if(_literal.empty())
      return;

    _bw.put_bits(_repeats_len, 0); /* Put literal makrer */
    _bw.put_bits(_repeats_len, _literal.size()); /* Put length of literal after marker */
    for(auto s : _literal)
    {
      _bw.put_bits(_symbol_len, s);
    }
    _literal.clear();
  }

  BitReader _br;
  BitWriter _bw;
  std::vector<uint64_t> _literal; /* Temporary buffer which contains escape sequence */
  uint8_t _repeats_len;           /* In bits */
  uint8_t _symbol_len;            /* In bits */
};

int main(int argc, char const *argv[])
{
  auto data = read_file_by_args(argc, argv, "a.txt");
  if(!data.has_value())
    return -1;
  
  RLEPack packer;

  packer.pack(data.value(), 4, 4);
  write_file_by_args(argc, argv, packer.get_result(), "a.bin");

  return 0;
}
