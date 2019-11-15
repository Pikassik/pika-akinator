#include <vector>
#include <string_view>
#include <string>

class AkinatorTree {
 public:
  struct Node {
    std::string_view string;
    size_t left   = 0; // true  subtree
    size_t right  = 0; // false subtree
    size_t parent = 0;
  };

  void ReadTree(const std::string& filename);
  const std::string& GetString();
  const std::vector<Node>& GetTree();;
  void Reserve(std::FILE* file);
  void BuildTree(std::FILE* file);
  inline bool IsLeaf(const Node& node) const;
 private:
  std::vector<Node> tree_;
  std::string nodes_strings_;
};