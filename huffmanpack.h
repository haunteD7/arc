#pragma once

#include "bitreader.h"
#include "bitwriter.h"
#include "huffman.h"

#include <memory>
#include <bitset>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_map>

static auto calculate_probabilities(const std::vector<uint8_t>& data)
{
  std::array<double, 256> result = { 0 };

  double rv_len = 1.f / static_cast<double>(data.size());
  for(auto sym : data)
  {
    result[sym] += rv_len;
  }

  return result;
}
static double calculate_entropy(const std::vector<uint8_t>& data) 
{
  double result = 0;
  auto probabilities = calculate_probabilities(data);
  for(const auto prob : probabilities)
  {
    if(prob > 0)
      result += prob * -std::log2(prob);
  }

  return result;
}

class HuffmanPack
{
public:
  std::vector<uint8_t> pack(std::vector<uint8_t> data, uint8_t symbol_len_bits)
  {
    if(data.empty())
      return {};
    
    size_t len = data.size() * 8 / symbol_len_bits;
    _br.set_data(std::move(data));
    
    /* Build dictionary */
    calculate_probabilities(symbol_len_bits, len);
    auto huffman_tree_top = std::move(build_huffman_tree());
    _dict.clear();
    build_dictionary(huffman_tree_top.get());
    
    /* Encode */
    /* Header */
    _bw.put_bits(header_len_len_bits, 0ULL); /* Header size */
    size_t data_len_pos = _bw.get_pos();
    _bw.put_bits(data_len_len_bits, 0ULL); /* Data size */
    for(const auto [sym, info] : _dict)
    {
      _bw.put_bits(symbol_len_bits, sym);
      _bw.put_bits(encoded_symbol_len_len, info.len);
    }
    size_t header_len = _bw.get_len();
    _bw.set_pos(0);
    _bw.put_bits(header_len_len_bits, header_len); /* Put header size */
    _bw.move_forward(header_len);
    /* Data */
    while(!_br.eof())
    {
      OgSymbol sym = _br.get_bits<OgSymbol>(symbol_len_bits);
      EcSymbolInfo sym_info = _dict[sym];
      _bw.put_bits(sym_info.len, sym_info.code);
    }
    _bw.set_pos(data_len_pos);
    _bw.put_bits(data_len_len_bits, _bw.get_len()); /* Put data size */

    return _bw.take_data();
  }
private:
  void calculate_probabilities(uint8_t symbol_len, size_t len)
  {
    double rv_len = 1.f / static_cast<double>(len);
    while(!_br.eof())
    {
      OgSymbol sym = _br.get_bits<OgSymbol>(symbol_len);
      _probabilities[sym] += rv_len;
    }
    _br.set_pos(0);
  }
  std::unique_ptr<HNode> build_huffman_tree()
  {
    auto cmp = [](const HNode* left, const HNode* right){
      return left->probability > right->probability;
    };
    
    std::priority_queue<HNode*, std::vector<HNode*>, decltype(cmp)> nodes(cmp);
    for(auto [sym, prob] : _probabilities) 
    {
      auto node = new HNode();
      node->symbol = sym;
      node->probability = prob;
      nodes.push(node);
    }
    
    while(nodes.size() > 1)
    {
      HNode* left = nodes.top();
      nodes.pop();
      
      HNode* right = nodes.top();
      nodes.pop();
      
      HNode* root = new HNode();
      root->left = left;
      root->right = right;
      root->probability = left->probability + right->probability;
      
      nodes.push(root);
    }

    return std::unique_ptr<HNode>(nodes.top());
  }
  void build_dictionary(HNode* node)
  {
    std::vector<OgSymbolInfo> symbols;

    build_lengths(node, symbols);
    std::ranges::sort(symbols, [](const OgSymbolInfo& l, const OgSymbolInfo& r) {
      if(l.ec_len == r.ec_len)
        return l.code < r.code;
        
      return l.ec_len < r.ec_len;
    });

    EcSymbol code = 0;
    uint8_t prev_len = 0;
    for(auto sym : symbols)
    {
      code <<= (sym.ec_len - prev_len);

      _dict[sym.code] = { code, sym.ec_len };
      
      code++;
      prev_len = sym.ec_len;
    }
  }
  void build_lengths(HNode* node, std::vector<OgSymbolInfo>& symbols, uint8_t depth = 0)
  {
    if(!node->left && !node->right) /* If it's last child */
    {
      symbols.push_back({ node->symbol, depth == (uint8_t)0 ? (uint8_t)1 : depth });
      return;
    }

    build_lengths(node->left, symbols, depth + 1);
    build_lengths(node->right, symbols, depth + 1);
  }
  BitReader _br;
  BitWriter _bw;
  
  std::unordered_map<OgSymbol, double> _probabilities;
  std::unordered_map<OgSymbol, EcSymbolInfo> _dict;
};

