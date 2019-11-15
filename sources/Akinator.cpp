#include <Akinator.h>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <zconf.h>
#include <filesystem>

struct PropertyNode {
    size_t index;
    bool property;
};

static const size_t kNodesReserve = 16;
static const size_t kStringReserve = 128;
static constexpr std::string_view kUnknown = "unknown";

void Akinator::ReadFile(const std::string& filename) {
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

void Akinator::WriteFile(const std::string& filename) const {
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
    fwrite(tree_.at(index).string.data(), 1,
           tree_.at(index).string.size(), file);
    fputc('\"', file);
  };

  push(0);
  while (!stack.empty()) {
    if (IsLeaf(tree_.at(stack.back()))) {
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

void Akinator::InteractiveMode() {
  while (true) {
    std::printf("Available modes:\n"
                "traversal\n"
                "difference\n"
                "write tree\n"
                "show tree\n"
                "Write one to start (or exit):");
    ReadLine();

    if        (input_buffer_ == "traversal") {
      TraversalMode();
    } else if (input_buffer_ == "difference") {
      DifferenceMode();
    } else if (input_buffer_ == "write tree") {
      WriteTreeMode();
    } else if (input_buffer_ == "show tree") {

    } else if (input_buffer_ == "exit") {
      return;
    }
  }
}

void Akinator::TraversalMode() {
  size_t current_node = 0;
  while (true) {
    if (!IsLeaf(tree_.at(current_node))) {
      std::printf("%.*s? (y or n):", tree_.at(current_node).string.size(),
                                   tree_.at(current_node).string.data());
      ReadLine();
      if (input_buffer_ == "yes") {
        current_node = tree_.at(current_node).left;
      } else if (input_buffer_ == "no") {
        current_node = tree_.at(current_node).right;
      } else {
        std::printf("Please, answer y or n");
      }
      continue;
    }

    std::printf("Is it %.*s?\n", tree_.at(current_node).string.size(),
                                 tree_.at(current_node).string.data());
    ReadLine();
    if (input_buffer_ == "y") {
      return;
    } else if (input_buffer_ == "n") {
      UpdateTree(current_node);
      return;
    } else {
      std::printf("Please, answer y or n\n");
      continue;
    }
  }
}

void Akinator::DifferenceMode() {
  auto find_node = [this]() {
    auto it = ReadAndFindCharacter();
    if (it == tree_.end())
      std::printf("there is no this character\n");
    return it;
  };

  std::printf("Enter first character:");
  auto first_node_it = find_node();
  if (first_node_it == tree_.cend()) return;
  size_t first_node = std::distance(tree_.cbegin(), first_node_it);

  std::printf("Enter second character:");
  auto second_node_it = find_node();
  if (second_node_it == tree_.cend()) return;
  size_t second_node = std::distance(tree_.cbegin(), second_node_it);

  size_t lca = LCA(first_node, second_node);

  std::vector<PropertyNode> properties_stack;
  properties_stack.reserve(tree_.size() / 2 + 1); // ne pravda

  CollectProperties(lca, 0, properties_stack);
  std::printf("Common properties: ");
  PrintProperties(properties_stack);

  CollectProperties(first_node, lca, properties_stack);
  std::printf("First characters' unique properties: ");
  PrintProperties(properties_stack);

  CollectProperties(second_node, lca, properties_stack);
  std::printf("Second characters' unique properties: ");
  PrintProperties(properties_stack);
}

void Akinator::Reserve(std::FILE* file) {
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

void Akinator::BuildTree(std::FILE* file) {

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
    tree_.at(traversal_stack.back().index).string =
      static_cast<std::string_view>(nodes_strings_).
      substr(begin_index, nodes_strings_.size() - begin_index);
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
      tree_.push_back({"", 0, 0,
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

std::vector<Akinator::Node>::const_iterator Akinator::ReadAndFindCharacter() {
  ReadLine();
  return std::find_if(tree_.begin(), tree_.end(),
                      [this](const Node& node) {
    return std::string_view(input_buffer_) == node.string &&
           IsLeaf(node);
  });
}

size_t Akinator::NodeDepth(size_t node) const {
  size_t depth = 0;
  while (node != 0) {
    node = tree_.at(node).parent;
    ++depth;
  }
  return depth;
}

size_t Akinator::LCA(size_t first_node, size_t second_node) const {
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

void
Akinator::PrintProperties(std::vector<PropertyNode>& stack) const {
  while (!stack.empty()) {
    if (!stack.back().property)
      std::printf("NOT ");
    assert(!tree_.at(stack.back().index).string.empty());
    std::printf("%.*s; ", tree_.at(stack.back().index).string.size() - 1,
                          tree_.at(stack.back().index).string.data());
    stack.pop_back();
  }
  std::putchar('\n');
  std::fflush(stdout);
}

void Akinator::CreateRoot() {
  nodes_strings_ += kUnknown;
  tree_.push_back({std::string_view(nodes_strings_).
                   substr(0, kUnknown.size()),
                   0, 0});
}

bool Akinator::IsLeaf(const Akinator::Node& node) const {
  return node.left == 0 && node.right == 0;
}

void Akinator::UpdateTree(size_t current_node) {
  std::printf("Please, write its correct attribute:");
  size_t begin = nodes_strings_.size();
  ReadLine();
  nodes_strings_ += input_buffer_;
  nodes_strings_.push_back('?');
  (tree_.at(tree_.at(current_node).parent).left == current_node ?
    tree_.at(tree_.at(current_node).parent).left :
    tree_.at(tree_.at(current_node).parent).right) = tree_.size();
  tree_.push_back({static_cast<std::string_view>(nodes_strings_).
                   substr(begin, input_buffer_.size() + 1),
                   tree_.size() + 1, current_node,
                   tree_.at(current_node).parent});
  tree_.at(current_node).parent = tree_.size() - 1;
  begin = nodes_strings_.size();
  std::printf("Please, write your character:");
  ReadLine();
  nodes_strings_ += input_buffer_;
  tree_.push_back({static_cast<std::string_view>(nodes_strings_).
                   substr(begin, input_buffer_.size()),
                   0, 0,
                   tree_.size() - 1});
}

void Akinator::ReadLine() {
  std::fflush(stdout);
  while (std::isspace(std::cin.peek())) std::cin.get();
  std::getline(std::cin, input_buffer_);
}

void Akinator::CollectProperties(size_t from,
                                size_t to,
                                std::vector<PropertyNode>& stack) const {
  auto is_true_property = [this, &stack](size_t index) {
    return tree_.at(tree_.at(index).parent).left == index;
  };

  stack.resize(0);
  if (from != to)
    stack.push_back({tree_.at(from).parent, is_true_property(from)});
  while (stack.back().index != to) {
    stack.push_back({tree_.at(stack.back().index).parent,
                     is_true_property(stack.back().index)});
  }
}

void Akinator::WriteTreeMode() {
  std::printf("Please, write filename for tree:");
  ReadLine();
  input_buffer_ += ".dot";
  FILE* dot_file = fopen((input_buffer_).c_str(), "w");
  std::fprintf(dot_file,
    "digraph akitree {\n"
    "graph [nodesep=0.5];\n"
    "node [fontname=\"Helvetica\", color=\"Lavender\", style=\"filled\"];\n");
  for (size_t i = 0; i < tree_.size(); ++i) {
    std::fprintf(dot_file,
      "node%zu [label=\"%.*s\"];\n",
      i, tree_.at(i).string.size(), tree_.at(i).string.data());
  }

  for (size_t i = 0; i < tree_.size(); ++i) {
    if (!IsLeaf(tree_.at(i))) {
      std::fprintf(dot_file,
        "edge [color=\"Green\"];\n"
        "node%zu -> node%zu;\n",
        i, tree_.at(i).left);
      std::fprintf(dot_file,
        "edge [color=\"Red\"];\n"
        "node%zu -> node%zu;\n",
        i, tree_.at(i).right);
    }
  }
  std::fwrite("}\n", 2, 1, dot_file);
  fclose(dot_file);
  system("dot tmp.dot -Tpng -o tmp.png");
  system("pix tmp.png &> /dev/null");
}
