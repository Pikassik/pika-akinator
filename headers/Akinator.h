#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <cstdio>
#include <iostream>

class Akinator {
 public:
  void ReadFile(const std::string& filename);
  void WriteFile(const std::string& filename) const;
  void InteractiveMode();
  void TraversalMode();
 private:
  struct Node {
    std::string_view string;
    size_t left = 0;
    size_t right = 0;
    size_t parent = 0;
  };

  void Reserve(std::FILE* file);
  void BuildTree(std::FILE* file);
  void UpdateTree(size_t current_node);
  void CreateRoot();
  inline bool IsLeaf(const Node& node) const;
  std::vector<Node> tree_;
  std::string nodes_strings_;
  std::string input_buffer_;
};