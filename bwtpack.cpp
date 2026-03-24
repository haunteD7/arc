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
    _result.clear();
    _shifts.clear();
    
    if(data.empty())
      return;

    _data.insert(_data.end(), data.begin(), data.end());
    _data.insert(_data.end(), data.begin(), data.end());
    size_t data_size = data.size();
    _result.resize(data_size + 4);

    _shifts.resize(data_size);
    for(size_t i = 0; i < data_size; i++)
    {
      _shifts[i] = { i };
    }
    /* Lexicographical sort */
    auto cmp = [&](const Shift& l, const Shift& r) -> bool
    {
      for (size_t i = 0; i < data_size; i+=8)
       {
        uint8_t bl = get_block(i + l.shift);
        uint8_t br = get_block(i + r.shift);

        if (bl < br) return true;   
        if (bl > br) return false;  
      }
      return false; 
    };
    std::ranges::sort(_shifts, cmp);
    size_t og_idx;
    for(size_t i = 0; i < data_size; i++)
    {
      if(_shifts[i].shift == 0)
      {
        og_idx = i;
        break;
      }
    }
    for(size_t i = 0; i < data_size; i++)
    {
      _result[i] = _data[_shifts[i].shift + data_size - 1];
    }
    std::memcpy(&_result[data_size], &og_idx, 4);
  }
  const std::vector<uint8_t>& get_result() const { return _result; }
private:
  uint64_t get_block(size_t idx)
  {
    uint64_t result;
    std::memcpy(&result, &_data[idx], sizeof(result));
    return result;
  }
  std::vector<Shift> _shifts;

  std::vector<uint8_t> _result;
  std::vector<uint8_t> _data;
};

int main(int argc, char const *argv[])
{
  auto data = read_file_by_args(argc, argv, "a.txt");
  if(!data.has_value())
    return -1;
  
  BWTUnpack packer;
  const auto start = std::chrono::steady_clock::now();
  packer.unpack(data.value());
  const auto end = std::chrono::steady_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << duration.count() << " ms\n";
  
  write_file_by_args(argc, argv, packer.get_result(), "a.bin");
  return 0;
}