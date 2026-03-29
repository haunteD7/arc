#include "bwtpack.h"
#include "mtfpack.h"
#include "huffmanpack.h"
#include "lzsspack.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

std::vector<uint8_t> load_file(std::string_view path)
{
  std::ifstream in_fs(path.data(), std::ios::binary);
  if(!in_fs.is_open())
    throw std::runtime_error("Cannot open file");

  std::error_code ec;
  size_t data_size = std::filesystem::file_size(path, ec);
  if (ec)
    throw std::runtime_error("Cannot get file size");

  std::vector<uint8_t> data(data_size);
  if(!in_fs.read((char*)data.data(), data_size))
    throw std::runtime_error("Cannot read file");

  return data;
}
void bwt_mtf_test()
{
  std::cout << "Entropy dependence on block size for bwt mtf, enter file name (type <skip> to skip): ";
  std::string load_path;
  std::cin >> load_path;
  if(load_path == "skip")
    return;

  auto data = load_file(load_path);
  constexpr size_t step = 64;
  constexpr size_t iterations = 64;

  std::stringstream ss;
  ss << "Block size;Entropy\n";
  for(size_t i = 1; i <= iterations; i++)  
  {
    size_t block_size = step * i;

    BWTPack bwt;
    MTFPack mtf;

    auto d = bwt.pack(data.begin(), data.end(), block_size);
    d = mtf.pack(std::move(d), 8);

    ss << block_size << ";" << calculate_entropy(d) << "\n";
    std::cout << (float)(i) / (float)iterations * 100.f << " %\n";
  }
  ss.seekp(0, std::ios::end);      
  std::ofstream out_fs("bwt mtf entropy.csv");
  out_fs.write(ss.str().c_str(), ss.tellp());
}
void lzss_test()
{
  std::cout << "LZSS compression test, enter file name (type <skip> to skip): ";
  std::string load_path;
  std::cin >> load_path;
  if(load_path == "skip")
    return;

  auto data = load_file(load_path);

  constexpr size_t step = 8196;
  constexpr size_t iterations = 64;

  std::stringstream ss;
  ss << "Buffer size;Compression\n";
  for(size_t i = 1; i <= iterations; i++)  
  {
    size_t buffer_size = i * step;
    
    LZSSPack lzss;
    float compression = (float)data.size() / (float)lzss.pack(data.begin(), data.end(), buffer_size).size();
    ss << buffer_size << ";" << compression << "\n";
    std::cout << (float)(i) / (float)iterations * 100.f << " %\n";
  }
  ss.seekp(0, std::ios::end);      
  std::ofstream out_fs("lzss.csv");
  out_fs.write(ss.str().c_str(), ss.tellp());
}
void lzw_test()
{
  std::cout << "LZW compression test, enter file name (type <skip> to skip): ";
  std::string load_path;
  std::cin >> load_path;
  if(load_path == "skip")
    return;

  auto data = load_file(load_path);

  constexpr size_t step = 8196;
  constexpr size_t iterations = 64;

  std::stringstream ss;
  ss << "Dictionary size;Compression\n";
  for(size_t i = 1; i <= iterations; i++)  
  {
    size_t dictionary_size = i * step;
    
    LZSSPack lzss;
    float compression = (float)data.size() / (float)lzss.pack(data.begin(), data.end(), dictionary_size).size();
    ss << dictionary_size << ";" << compression << "\n";
    std::cout << (float)i / (float)iterations * 100.f << " %\n";
  }
  ss.seekp(0, std::ios::end);      
  std::ofstream out_fs("lzw.csv");
  out_fs.write(ss.str().c_str(), ss.tellp());
}

int main(int argc, char const *argv[])
{
  bwt_mtf_test();
  lzss_test();
  lzw_test();

  return 0;
}
