#pragma once
#include <vector>
#include <string_view>
#include <string>

extern const size_t kStringReserve;

class AkinatorTree {
 public:
  struct Node {
    std::string_view string;
    size_t left   = 0; // true  subtree
    size_t right  = 0; // false subtree
    size_t parent = 0;
  };

  void ReadTree(const std::string& filename);
  void WriteTree(const std::string& filename) const;
  void UpdateTree(size_t current_node,
                  const std::string& property,
                  const std::string& new_node);

  const std::string& GetString() const;
  const std::vector<Node>& GetTree() const;
  inline bool IsLeaf(size_t node) const;
  size_t LCA(size_t first_node, size_t second_node) const;
  size_t NodeDepth(size_t node) const;
 private:
  void CreateRoot();
  void Reserve(std::FILE* file);
  void BuildTree(std::FILE* file);
  std::vector<Node> tree_;
  std::string nodes_strings_;
};
