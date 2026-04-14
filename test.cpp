#include "bwtpack.h"
#include "mtfpack.h"
#include "huffmanpack.h"
#include "lzsspack.h"
#include "lzwpack.h"

#include <fstream>
#include <chrono>
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
  constexpr size_t start_val = 1024; /* 1 KB */
  constexpr size_t iterations = 10;

  std::stringstream ss;
  ss << "Block size;Entropy;Time\n";
  for(size_t i = 0; i <= iterations; i++)  
  {
    size_t size = start_val * std::pow(2, i);
    
    BWTPack bwt;
    MTFPack mtf;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto d = bwt.pack(data.begin(), data.end(), size);
    d = mtf.pack(std::move(d), 8);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    ss << size << ";" << calculate_entropy(d) << ";" << duration.count() << "\n";
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

  constexpr size_t start_val = 1024; /* 1 KB */
  constexpr size_t iterations = 10;

  std::stringstream ss;
  ss << "Buffer size;Compression;Time\n";
  for(size_t i = 0; i <= iterations; i++)  
  {
    size_t size = start_val * std::pow(2, i);
    
    LZSSPack lzss;
    auto start = std::chrono::high_resolution_clock::now();
    float compression = (float)data.size() / (float)lzss.pack(data.begin(), data.end(), size).size();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    ss << size << ";" << compression << ";" << duration.count() << "\n";
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

  constexpr size_t start_val = 1024; /* 1 KB */
  constexpr size_t iterations = 10;

  std::stringstream ss;
  ss << "Dictionary size;Compression;Time\n";
  for(size_t i = 0; i <= iterations; i++)  
  {
    size_t size = start_val * std::pow(2, i);
    
    LZWPack lzw;

    auto start = std::chrono::high_resolution_clock::now();
    float compression = (float)data.size() / (float)lzw.pack(data.begin(), data.end(), size).size();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    ss << size << ";" << compression << ";" << duration.count() << "\n";
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
