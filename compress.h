#pragma once

#include "bwtpack.h"
#include "huffmanpack.h"
#include "rlepack.h"
#include "mtfpack.h"
#include "lzwpack.h"
#include "lzsspack.h"

static std::string compressor_names[] =
{
  "HA",
  "Run-length encoding (RLE)",
  "BWT + RLE",
  "BWT + MTF + HA",
  "BWT + MTF + RLE + HA",
  "LZSS",
  "LZSS + HA",
  "LZW",
  "LZW + HA",
};

void compress(int compressor, std::vector<uint8_t> &data)
{
  switch (compressor)
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
  case 3: /* BWT + RLE */
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
    data = lzss.pack(data, 4095);
    break;
  }
  case 7: /* LZSS + HA */
  {
    LZSSPack lzss;
    data = lzss.pack(data, 4095);
    HuffmanPack ha;
    data = ha.pack(std::move(data), 8);
    break;
  }
  case 8: /* LZW */
  {
    LZWPack lzw;
    data = lzw.pack(data, 30720);
    break;
  }
  case 9: /* LZW + HA */
  {
    LZWPack lzw;
    data = lzw.pack(data, 30720);
    HuffmanPack ha;
    data = ha.pack(std::move(data), 8);
    break;
  }
  }
}