#include <iostream>
#include <filesystem>
#include <fstream>

#include "bwtpack.h"
#include "bwtunpack.h"
#include "huffmanpack.h"
#include "huffmanunpack.h"
#include "rlepack.h"
#include "rleunpack.h"
#include "mtfpack.h"
#include "mtfunpack.h"
#include "lzwpack.h"
#include "lzwunpack.h"
#include "lzsspack.h"
#include "lzssunpack.h"

int main(int argc, char const *argv[])
{
  std::cout << R"(Select compressor:
1. HA
2. Run-length encoding (RLE)
3. BWT + RLE
4. BWT + MTF + HA
5. BWT + MTF + RLE + HA
6. LZSS
7. LZSS + HA
8. LZW
9. LZW + HA

)";
  int compressor;
  if (!(std::cin >> compressor) || compressor < 1 || compressor > 9)
  {
    std::cerr << "Invalid compressor\n";
    return -1;
  }

  std::cout << "Compress / Decompress [c/d]: ";
  char cd;
  if (!(std::cin >> cd) || (cd != 'c' && cd != 'd'))
  {
    std::cerr << "Invalid mode\n";
    return -1;
  }

  std::cout << "Load path: ";
  std::string load_path;
  std::cin >> load_path;
  std::cout << "Save path: ";
  std::string save_path;
  std::cin >> save_path;

  std::ifstream in_fs(load_path, std::ios::binary);
  if(!in_fs.is_open())
  {
    std::cerr << "Can't open file to read " << load_path << "\n";
    return -1;
  }
  std::error_code ec;
  size_t data_size = std::filesystem::file_size(load_path, ec);
  if (ec)
  {
    std::cerr << "Failed to get file size\n";
    return -1;
  }
  std::vector<uint8_t> data(data_size);
  if(!in_fs.read((char*)data.data(), data_size))
  {
    std::cerr << "Error reading file\n";
    return -1;
  }

  try
  {
    if(cd == 'c')
    {
      switch(compressor)
      {
        case 1: /* Huffman */
        {
          HuffmanPack ha;
          data = ha.pack(std::move(data), 8);
          break;
        }
        case 2: /* RLE */
        {
          RLEPack rle;
          data = rle.pack(data.begin(), data.end(), 1, 1);
          break;
        }
        case 3:  /* BWT + RLE */
        {
          BWTPack bwt;
          data = bwt.pack(data.begin(), data.end(), 256);
          RLEPack rle;
          data = rle.pack(data.begin(), data.end(), 1, 1);
          break;
        }
        case 4: /* BWT + MTF + HA */
        {
          BWTPack bwt;
          data = bwt.pack(data.begin(), data.end(), 256);
          MTFPack mtf;
          data = mtf.pack(std::move(data), 8);
          HuffmanPack ha;
          data = ha.pack(std::move(data), 8);
          break;
        }
        case 5: /* 5. BWT + MTF + RLE + HA */
        {
          BWTPack bwt;
          data = bwt.pack(data.begin(), data.end(), 256);
          MTFPack mtf;
          data = mtf.pack(std::move(data), 8);
          RLEPack rle;
          data = rle.pack(data.begin(), data.end(), 1, 1);
          HuffmanPack ha;
          data = ha.pack(std::move(data), 8);
          break;
        }
        case 6: /* LZSS */
        {
          LZSSPack lzss;
          data = lzss.pack(data.begin(), data.end());
          break;
        }
        case 7: /* LZSS + HA */
        {
          LZSSPack lzss;
          data = lzss.pack(data.begin(), data.end());
          HuffmanPack ha;
          data = ha.pack(std::move(data), 8);
          break;
        }
        case 8: /* LZW */
        {
          LZWPack lzw;
          data = lzw.pack(data.begin(), data.end());
          break;
        }
        case 9: /* LZW + HA */
        {
          LZWPack lzw;
          data = lzw.pack(data.begin(), data.end());
          HuffmanPack ha;
          data = ha.pack(std::move(data), 8);
          break;
        }
      }
    }
    else if(cd == 'd')
    {
      switch(compressor)
      {
        case 1: /* Huffman */
        {
          HuffmanUnpack ha;
          data = ha.unpack(std::move(data), 8);
          break;
        }
        case 2: /* RLE */
        {
          RLEUnpack rle;
          data = rle.unpack(data.begin(), data.end());
          break;
        }
        case 3:  /* BWT + RLE */
        {
          RLEUnpack rle;
          data = rle.unpack(data.begin(), data.end());
          BWTUnpack bwt;
          data = bwt.unpack(data.begin(), data.end());
          break;
        }
        case 4: /* BWT + MTF + HA */
        {
          HuffmanUnpack ha;
          data = ha.unpack(std::move(data), 8);
          MTFUnpack mtf;
          data = mtf.unpack(std::move(data), 8);
          BWTUnpack bwt;
          data = bwt.unpack(data.begin(), data.end());
          break;
        }
        case 5: /* 5. BWT + MTF + RLE + HA */
        {
          HuffmanUnpack ha;
          data = ha.unpack(std::move(data), 8);
          RLEUnpack rle;
          data = rle.unpack(data.begin(), data.end());
          MTFUnpack mtf;
          data = mtf.unpack(std::move(data), 8);
          BWTUnpack bwt;
          data = bwt.unpack(data.begin(), data.end());
          break;
        }
        case 6: /* LZSS */
        {
          LZSSUnpack lzss;
          data = lzss.unpack(data.begin(), data.end());
          break;
        }
        case 7: /* LZSS + HA */
        {
          HuffmanUnpack ha;
          data = ha.unpack(std::move(data), 8);
          LZSSUnpack lzss;
          data = lzss.unpack(data.begin(), data.end());
          break;
        }
        case 8: /* LZW */
        {
          LZWUnpack lzw;
          data = lzw.unpack(data.begin(), data.end());
          break;
        }
        case 9: /* LZW + HA */
        {
          HuffmanUnpack ha;
          data = ha.unpack(std::move(data), 8);
          LZWUnpack lzw;
          data = lzw.unpack(data.begin(), data.end());
          break;
        }
      }
    }
  }
  catch(const std::exception& e)
  {
    std::cerr << "Error: file is damaged\n";
    return -1;
  }

  std::ofstream out_fs(save_path, std::ios::binary);
  if(!out_fs.is_open())
  {
    std::cerr << "Can't open file to write " << save_path << "\n";
    return -1;
  }
  if(!out_fs.write(reinterpret_cast<const char*>(data.data()), data.size()))
  {
    std::cerr << "Error writing file\n";
    return -1;
  }
  return 0;
}
