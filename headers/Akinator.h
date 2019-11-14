#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <cstdio>
#include <iostream>

struct PropertyNode;

class Akinator {
 public:
  void ReadFile(const std::string& filename);
  void WriteFile(const std::string& filename) const;
  void InteractiveMode();
  void TraversalMode();
  void DifferenceMode();
 private:
  struct Node {
    std::string_view string;
    size_t left   = 0; // true  subtree
    size_t right  = 0; // false subtree
    size_t parent = 0;
  };

  void Reserve(std::FILE* file);
  void BuildTree(std::FILE* file);
  void UpdateTree(size_t current_node);
  std::vector<Node>::const_iterator ReadAndFindCharacter();
  size_t NodeDepth(size_t node) const;
  size_t LCA(size_t first_node, size_t second_node) const;
  void PrintProperties(std::vector<PropertyNode>& stack) const;
  void CreateRoot();
  void ReadLine();
  void CollectProperties(size_t from,
                        size_t to,
                        std::vector<PropertyNode>& stack) const;
  inline bool IsLeaf(const Node& node) const;
  std::vector<Node> tree_;
  std::string nodes_strings_;
  std::string input_buffer_;
};