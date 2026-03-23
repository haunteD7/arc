#include "bitreader.h"
#include "bitwriter.h"
#include "utils.h"

class MTFUnpack
{
public:
  void unpack(const std::vector<uint8_t>& data, uint8_t symbol_len)
  {
    if(data.empty())
      return;
    _br.set_data(data);
    
    std::vector<uint64_t> dict(1 << symbol_len);
    for(size_t i = 0; i < dict.size(); i++)
      dict[i] = i;

    while(!_br.eof())
    {
      uint64_t idx = _br.get_bits<uint64_t>(symbol_len);
      uint64_t sym = dict[idx];
      _bw.put_bits(symbol_len, sym);

      /* Move to front */
      for(size_t i = idx; i > 0; i--)
        dict[i] = dict[i - 1];
      dict[0] = sym;
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
  
  MTFUnpack unpacker;
  unpacker.unpack(data.value(), 8);
  
  write_file_by_args(argc, argv, unpacker.get_result(), "a.txt");
  return 0;
}