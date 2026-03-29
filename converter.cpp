#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstdint>
#include <fstream>
#include <iostream>

enum class ImageType : uint8_t
{
  Color = 0,
  Grey,
  BlackWhite,
};

int main(int argc, char const *argv[])
{
  int width, height, channels;
  uint8_t* color = stbi_load("color.bmp", &width, &height, &channels, 0);
  uint8_t* grey = stbi_load("grey.bmp", &width, &height, &channels, 0);
  uint8_t* bw = stbi_load("bw.bmp", &width, &height, &channels, 0);

  std::ofstream color_fs("color.raw", std::ios::binary);
  color_fs.put(static_cast<uint8_t>(ImageType::Color));
  color_fs.write((const char*)color, 1920 * 1080 * 3);

  std::ofstream grey_fs("grey.raw", std::ios::binary);
  grey_fs.put(static_cast<uint8_t>(ImageType::Grey));
  grey_fs.write((const char*)grey, 1920 * 1080 * 3);
  
  std::ofstream bw_fs("bw.raw", std::ios::binary);
  bw_fs.put(static_cast<uint8_t>(ImageType::BlackWhite));
  bw_fs.write((const char*)bw, 1920 * 1080 * 3);
  return 0;
}
