#include "bitreader.h"
#include "bitwriter.h"
#include "utils.h"

#include <ranges>
#include <chrono>
#include <algorithm>
#include <cstring>

class BWTUnpack
{
public:
  struct Shift
  {
    size_t shift;
  };

  void unpack(const std::vector<uint8_t>& data)
  {
    if(data.empty())
      return;

    size_t data_size = data.size() - 4;
    size_t og_idx;
    std::memcpy(&og_idx, &data[data_size], 4);

    std::vector<uint8_t> first = data;
    first.resize(data_size);
    std::ranges::sort(first);
  }
  const std::vector<uint8_t>& get_result() const { return _result; }
private:
  std::vector<uint8_t> _result;
};

int main(int argc, char const *argv[])
{
  auto data = read_file_by_args(argc, argv, "a.txt");
  if(!data.has_value())
    return -1;
  
  BWTUnpack unpacker;
  const auto start = std::chrono::steady_clock::now();
  unpacker.unpack(data.value());
  const auto end = std::chrono::steady_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << duration.count() << " ms\n";
  
  write_file_by_args(argc, argv, unpacker.get_result(), "a.bin");
  return 0;
}