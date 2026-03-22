#include "utils.h"
#include "bitreader.h"
#include "bitwriter.h"

class RLEUnpack
{
public:
  void unpack(const std::vector<uint8_t>& data)
  {
    if(data.empty())
      return;
      
    uint8_t repeats_len;
    uint8_t symbol_len;
    uint8_t padding;
    {
      BitReader header_br;
      header_br.set_data(data);

      repeats_len = _br.get_bits<uint8_t>(5) + 1;
      symbol_len = _br.get_bits<uint8_t>(5) + 1;
      padding = _br.get_bits<size_t>(3);

      _br.set_data(data, padding);
      _br.move_forward(header_br.len());
    }

    while(!_br.eof())
    {
      size_t repeats = _br.get_bits<size_t>(repeats_len);
      if(repeats == 0)
      {
        size_t literal_len = _br.get_bits<size_t>(repeats_len);
        for(size_t i = 0; i < literal_len; i++)
        {
          uint64_t sym = _br.get_bits<uint64_t>(symbol_len);
          _bw.put_bits(symbol_len, sym);
        }
      }
      else
      {
        uint64_t sym = _br.get_bits<uint64_t>(symbol_len);
        for(size_t i = 0; i < repeats; i++)
        {
          _bw.put_bits(symbol_len, sym);
        }
      }
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

  RLEUnpack unpacker;
  unpacker.unpack(data.value());
  
  write_file_by_args(argc, argv, unpacker.get_result(), "a.txt");

  return 0;
}

