#include "bitreader.h"
#include "bitwriter.h"
#include "utils.h"

class BWTPack
{
public:
  void pack(const std::vector<uint8_t>& data, uint8_t symbol_len)
  {
  }
  const std::vector<uint8_t>& get_result() { return _bw.get_data(); }
private:
  BitReader _br;
  BitWriter _bw;
};

int main(int argc, char const *argv[])
{
  auto data = read_file_by_args(argc, argv, "a.txt");
  if(!data.has_value())
    return -1;
  
  BWTPack packer;
  packer.pack(data.value(), 8);
  
  write_file_by_args(argc, argv, packer.get_result(), "a.bin");
  return 0;
}