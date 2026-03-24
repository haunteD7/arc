#include "utils.h"

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <iterator>
#include <stdexcept>

class LZWPack
{
public:
    template <std::random_access_iterator Iterator>
    void pack(Iterator begin, Iterator end, uint16_t max_dict_size = 4096)
    {
        _result.clear();
        if (begin == end) return;

        // Инициализация словаря: все 1-байтовые символы
        std::unordered_map<std::vector<uint8_t>, uint16_t> dict;
        for (uint16_t i = 0; i < 256; i++)
            dict[{static_cast<uint8_t>(i)}] = i;

        uint16_t dict_size = 256;
        std::vector<uint8_t> w;

        for (Iterator it = begin; it != end; ++it)
        {
            std::vector<uint8_t> k{*it};
            std::vector<uint8_t> wk = w;
            wk.push_back(*it);

            if (dict.count(wk))
            {
                w = wk;
            }
            else
            {
                write_code(dict[w]);
                if (dict_size < max_dict_size)
                    dict[wk] = dict_size++;
                w = k;
            }
        }

        if (!w.empty())
            write_code(dict[w]);
    }

    const std::vector<uint8_t>& get_result() const { return _result; }

private:
    void write_code(uint16_t code)
    {
        _result.push_back(static_cast<uint8_t>(code & 0xFF));
        _result.push_back(static_cast<uint8_t>((code >> 8) & 0xFF));
    }

    std::vector<uint8_t> _result;
};

int main(int argc, char const *argv[])
{
  auto data = read_file_by_args(argc, argv, "a.txt");
  if(!data.has_value())
    return -1;
  
  LZWPack packer;
  const auto start = std::chrono::steady_clock::now();
  packer.pack(data.value().begin(), data.value().end());
  const auto end = std::chrono::steady_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << duration.count() << " ms\n";
  
  write_file_by_args(argc, argv, packer.get_result(), "a.bin");
  return 0;
}