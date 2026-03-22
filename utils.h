#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <optional>

static std::optional<std::vector<uint8_t>> read_file_by_args(int argc, char const *argv[], std::string_view default_file)
{
  std::string input_file_name;
  if(argc < 2)
    input_file_name = default_file;
  else
    input_file_name = argv[1];
  
  std::ifstream input_fs(input_file_name, std::ios::binary);
  if(!input_fs.is_open())
  {
    std::cerr << "Can't open file to read " << input_file_name << "\n";
    return std::nullopt;
  }
  size_t file_size = std::filesystem::file_size(input_file_name);
  if(file_size == 0)
  {
    std::cout << "File is empty\n";
    return std::nullopt;
  }
  
  std::vector<uint8_t> data(file_size);
  input_fs.read(reinterpret_cast<char*>(data.data()), file_size);
  std::cout << "Has been read data of size " << file_size <<  " bytes" << "\n";
  
  return data;
}

static bool write_file_by_args(int argc, char const *argv[], const std::vector<uint8_t>& data, std::string_view default_file)
{
  std::string output_file_name;
  if(argc < 3)
    output_file_name = default_file;
  else
    output_file_name = argv[2];
  
  std::ofstream output_fs(output_file_name, std::ios::binary); 
  if(!output_fs.is_open())
  {
    std::cerr << "Can't open file to write " << output_file_name << "\n";
    return false;
  }
  
  output_fs.write(reinterpret_cast<const char*>(data.data()), data.size());
  std::cout << "Has been written data of size " << data.size() <<  " bytes" << "\n";

  return true;
}