#include "bitreader.h"
#include "bitwriter.h"
#include "utils.h"
#include "huffman.h"

#include <memory>
#include <iostream>
#include <cmath>
#include <queue>
#include <unordered_map>

class HuffmanPack
{
public:
  void pack(const std::vector<uint8_t>& data, uint8_t symbol_len)
  {
    if(data.empty())
      return;
    
    size_t len = data.size() * 8 / symbol_len;
    _br.set_data(data, len % 8);
    
    /* Build dictionary */
    calculate_probabilities(symbol_len, len);
    auto huffman_tree_top = std::move(build_huffman_tree());
    _dict.clear();
    build_dictionary(huffman_tree_top.get());
    
    /* Encode */
    /* Header */
    _bw.put_bits(header_len_len, 0); /* Header size */
    _bw.put_bits(symbol_len_len, symbol_len - 1);
    size_t padding_pos = _bw.get_pos();
    _bw.put_bits(3, 0); /* Padding */
    for(const auto [sym, info] : _dict)
    {
      _bw.put_bits(symbol_len, sym);
      _bw.put_bits(5, info.len - 1);
    }
    size_t header_len = _bw.len();
    _bw.set_pos(0);
    _bw.put_bits(header_len_len, _bw.len());
    _bw.move_forward(header_len);
    /* Data */
    while(!_br.eof())
    {
      OgSymbol sym = _br.get_bits<OgSymbol>(symbol_len);
      EcSymbolInfo sym_info = _dict[sym];
      _bw.put_bits(sym_info.len, sym_info.code);
    }
    _bw.set_pos(padding_pos);
    _bw.put_bits(3, _bw.get_padding()); /* Put padding */
  }
  double get_entropy() const 
  {
    double result = 0;
    for(const auto [sym, prob] : _probabilities)
    {
      double len = _dict.at(sym).len;
      result += len * -std::log2(prob);
    }

    return result;
  }
  const std::vector<uint8_t>& get_result() { return _bw.get_data(); }
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
  void build_dictionary(HNode* node, uint8_t depth = 0, EcSymbol symbol = 0)
  {
    if(!node->left && !node->right) /* If it's last child */
    {
      _dict[node->symbol] = { symbol, depth == (uint8_t)0 ? (uint8_t)1 : depth };
      return;
    }
  
    build_dictionary(node->left, depth + 1, symbol << 1); /* Left child (add 0) */
    build_dictionary(node->right, depth + 1, (symbol << 1) | 1); /* Right child (add 1) */
  }
  BitReader _br;
  BitWriter _bw;
  
  std::unordered_map<OgSymbol, double> _probabilities;
  std::unordered_map<OgSymbol, EcSymbolInfo> _dict;
};

int main(int argc, char const *argv[])
{
  auto data = read_file_by_args(argc, argv, "a.txt");
  if(!data.has_value())
    return -1;
  
  HuffmanPack packer;
  packer.pack(data.value(), 8);
  std::cout << packer.get_entropy() << "\n";
  
  write_file_by_args(argc, argv, packer.get_result(), "a.bin");
  return 0;
}