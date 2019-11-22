#include <AkinatorTree.h>
#include <filesystem>
#include <cassert>

extern constexpr size_t kNodesReserve = 16;
extern constexpr size_t kStringReserve = 128;
extern constexpr std::string_view kUnknown = "unknown";

void AkinatorTree::ReadTree(const std::string& filename) {
  tree_.resize(0);
  nodes_strings_.resize(0);
  if (!std::filesystem::exists(filename)) {
    tree_.reserve(kNodesReserve);
    nodes_strings_.reserve(kStringReserve);
    CreateRoot();
    return;
  }
  std::FILE* file = fopen(filename.c_str(), "r");
  assert(file != nullptr);
  Reserve(file);
  BuildTree(file);
  fclose(file);
}

void AkinatorTree::BuildTree(std::FILE* file) {

  struct TraversalNode {
    size_t index;
    bool is_visited;
  };

  char input_char = 0;
  std::vector<TraversalNode> traversal_stack;
  traversal_stack.reserve(tree_.size() / 2 + 1);

  auto read_string = [&]() {
    while (std::isspace(input_char = fgetc(file)));
    assert(input_char == '"');
    size_t begin_index = nodes_strings_.size();
    while ((input_char = fgetc(file)) != '"')
      nodes_strings_.push_back(input_char);
    tree_.at(traversal_stack.back().index).left_bound =  begin_index;
    tree_.at(traversal_stack.back().index).right_bound = nodes_strings_.size();
  };

  while (((input_char = fgetc(file)) != '{')) {
    if (input_char == -1) {
      CreateRoot();
      return;
    }
  }

  traversal_stack.push_back({tree_.size(), false});
  tree_.push_back({});
  read_string();

  while ((input_char = fgetc(file)) != EOF) {
    if (std::isspace(input_char)) continue;

    if (input_char == '{') {
      if (traversal_stack.back().is_visited) {
        tree_.at(traversal_stack.back().index).right = tree_.size();
      } else {
        tree_.at(traversal_stack.back().index).left = tree_.size();
      }

      traversal_stack.push_back({tree_.size(), false});
      tree_.push_back({0, 0, 0, 0,
                       traversal_stack.at(traversal_stack.size() - 2).index});

      read_string();
    }

    if (input_char == '}') {
      traversal_stack.pop_back();
      if (!traversal_stack.empty())
        traversal_stack.back().is_visited = true;
    }
  }
  traversal_stack.pop_back();
}

void AkinatorTree::CreateRoot() {
  nodes_strings_ += kUnknown;
  tree_.push_back({0, kUnknown.size(),
                   0, 0});
}

void AkinatorTree::WriteTree(const std::string& filename) const {
  std::FILE* file = fopen(filename.c_str(), "w");
  assert(file != nullptr);

  std::vector<int> visited(tree_.size(), 0);
  std::vector<size_t> stack;
  stack.reserve(tree_.size() / 2 + 1);

  auto pop = [&file, &visited, &stack]() {
    fputc('}', file);
    visited.at(stack.back()) = 1;
    stack.pop_back();
  };

  auto push = [this, &file, &stack](size_t index) {
    stack.push_back(index);
    fputc('{', file);
    fputc('\"', file);
    fwrite(NodeToString(index).data(), 1,
           NodeToString(index).size(), file);
    fputc('\"', file);
  };

  push(0);
  while (!stack.empty()) {
    if (IsLeaf(stack.back())) {
      pop();
      continue;
    }
    if (visited.at(tree_.at(stack.back()).left) &&
        visited.at(tree_.at(stack.back()).right)) {
      pop();
      continue;
    }
    if (!visited.at(tree_.at(stack.back()).left)) {
      push(tree_.at(stack.back()).left);
      continue;
    }
    if (!visited.at(tree_.at(stack.back()).right)) {
      push(tree_.at(stack.back()).right);
    }
  }
  fclose(file);
}

std::string_view AkinatorTree::NodeToString(size_t node) const {
    return static_cast<std::string_view>(nodes_strings_).
      substr(tree_.at(node).left_bound,
             tree_.at(node).right_bound - tree_.at(node).left_bound);
}

const std::string& AkinatorTree::GetString() const {
  return nodes_strings_;
}

const std::vector<AkinatorTree::Node>& AkinatorTree::GetTree() const {
  return tree_;
}

bool AkinatorTree::IsLeaf(size_t node) const {
  return tree_.at(node).left == 0 && tree_.at(node).right == 0;
}

void AkinatorTree::Reserve(std::FILE* file) {
  int input_char = 0;
  size_t quote_count = 0;
  size_t symbols_count = 0;
  bool is_string = false;
  while ((input_char = fgetc(file)) != EOF) {
    if (input_char == '"') {
      ++quote_count;
      is_string = !is_string;
      continue;
    }
    if (is_string)
      ++symbols_count;
  }
  fseek(file, 0, SEEK_SET);
  assert(quote_count % 2 == 0);
  tree_.reserve(quote_count / 2 + kNodesReserve);
  nodes_strings_.reserve(symbols_count + kStringReserve);
}

void AkinatorTree::UpdateTree(size_t current_node,
                              const std::string& property,
                              const std::string& new_node) {
  size_t begin = nodes_strings_.size();
  nodes_strings_ += property;
  nodes_strings_.push_back('?');
  (tree_.at(tree_.at(current_node).parent).left == current_node ?
    tree_.at(tree_.at(current_node).parent).left :
    tree_.at(tree_.at(current_node).parent).right) = tree_.size();


  tree_.push_back({begin , begin + property.size() + 1,
                   tree_.size() + 1, current_node,
                   tree_.at(current_node).parent});
  tree_.at(current_node).parent = tree_.size() - 1;
  begin = nodes_strings_.size();
  nodes_strings_ += new_node;
  tree_.push_back({begin, begin + new_node.size(),
                   0, 0,
                   tree_.size() - 1});
  if (tree_.size() == 3) {
  // hard code for first update
    std::swap(tree_.at(0), tree_.at(1));
    tree_.at(0).left = 2;
    tree_.at(0).right = 1;
    tree_.at(1).parent = 0;
    tree_.at(1).left = 0;
    tree_.at(2).parent = 0;
  }
}

size_t AkinatorTree::LCA(size_t first_node, size_t second_node) const {
  size_t first_depth = NodeDepth(first_node);
  size_t second_depth = NodeDepth(second_node);
  bool second_deeper = false;
  if (second_depth > first_depth) {
    second_deeper = true;
    std::swap(first_depth, second_depth);
    std::swap(first_node, second_node);
  }
  for (size_t i = 0; i < first_depth - second_depth; ++i) {
    first_node = tree_.at(first_node).parent;
  }
  if (second_deeper) std::swap(first_node, second_node);
  while (first_node != second_node) {
    first_node  = tree_.at(first_node).parent;
    second_node = tree_.at(second_node).parent;
  }
  return first_node;
}

size_t AkinatorTree::NodeDepth(size_t node) const {
  size_t depth = 0;
  while (node != 0) {
    node = tree_.at(node).parent;
    ++depth;
  }
  return depth;
}
