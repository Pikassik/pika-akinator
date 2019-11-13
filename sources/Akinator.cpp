#include <Akinator.h>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <experimental/filesystem>

static const size_t kNodesReserve = 16;
static const size_t kStringReserve = 128;
static constexpr std::string_view kUnknown = "unknown";

void Akinator::ReadFile(const std::string& filename) {
  tree_.resize(0);
  nodes_strings_.resize(0);
  if (std::experimental::filesystem::exists(filename)) {
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
    fwrite("}", 1, 1, file);
    visited.at(stack.back()) = 1;
    stack.pop_back();
  };

  auto push = [this, &file, &stack](size_t index) {
    stack.push_back(index);
    fwrite("{", 1, 1, file);
    fwrite("\"", 1, 1, file);
    fwrite(tree_.at(index).string.data(), 1,
           tree_.at(index).string.size(), file);
    fwrite("\"", 1, 1, file);
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
  size_t current_node = 0;
  while (true) {
    if (!IsLeaf(tree_.at(current_node))) {
      std::printf("%.*s (yes or no):", tree_.at(current_node).string.size(),
                                   tree_.at(current_node).string.data());
      while (std::isspace(std::cin.peek())) std::cin.get();
      std::getline(std::cin, input_buffer_);
      if (input_buffer_ == "y") {
        current_node = tree_.at(current_node).left;
      } else if (input_buffer_ == "n") {
        current_node = tree_.at(current_node).right;
      } else {
        std::cout << "Please, answer y or n";
      }
      continue;
    }

    std::printf("Is it %.*s?\n", tree_.at(current_node).string.size(),
                                 tree_.at(current_node).string.data());
    std::fflush(stdout);
    while (std::isspace(std::cin.peek())) std::cin.get();
    std::getline(std::cin, input_buffer_);
    if (input_buffer_ == "y") {
      printf("Easy.\n");
      return;
    } else if (input_buffer_ == "n") {
      UpdateTree(current_node);
      printf("Thank you!\n");
    } else {
      std::cout << "Please, answer y or n\n";
      continue;
    }
    break;
  }
}

void Akinator::TraversalMode() {
  size_t current_node = 0;
  while (true) {
    if (!IsLeaf(tree_.at(current_node))) {
      std::printf("%.*s? (yes or no):", tree_.at(current_node).string.size(),
                                   tree_.at(current_node).string.data());
      while (std::isspace(std::cin.peek())) std::cin.get();
      std::getline(std::cin, input_buffer_);
      if (input_buffer_ == "y") {
        current_node = tree_.at(current_node).left;
      } else if (input_buffer_ == "n") {
        current_node = tree_.at(current_node).right;
      } else {
        std::printf("Please, answer y or n");
      }
      continue;
    }

    std::printf("Is it %.*s?\n", tree_.at(current_node).string.size(),
                                 tree_.at(current_node).string.data());
    std::fflush(stdout);
    while (std::isspace(std::cin.peek())) std::cin.get();
    std::getline(std::cin, input_buffer_);
    if (input_buffer_ == "y") {
      return;
    } else if (input_buffer_ == "n") {
      UpdateTree(current_node);
      return;
    } else {
      std::cout << "Please, answer y or n\n";
      continue;
    }
  }
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
  printf("Please, write its correct attribute:");
  size_t begin = nodes_strings_.size();
  while (std::isspace(std::cin.peek())) std::cin.get();
  std::getline(std::cin, input_buffer_);
  nodes_strings_ += input_buffer_;
  nodes_strings_.push_back('?');
  (tree_.at(tree_.at(current_node).parent).left == current_node ?
    tree_.at(tree_.at(current_node).parent).left :
    tree_.at(tree_.at(current_node).parent).right) = tree_.size();
  tree_.push_back({static_cast<std::string_view>(nodes_strings_).
                   substr(begin, input_buffer_.size() + 1),
                   tree_.size() + 1, current_node, // ?
                   tree_.at(current_node).parent}); // here i need parent
  tree_.at(current_node).parent = tree_.size() - 1;
  begin = nodes_strings_.size();
  printf("Please, write your character:");
  while (std::isspace(std::cin.peek())) std::cin.get();
  std::getline(std::cin, input_buffer_);
  nodes_strings_ += input_buffer_;
  tree_.push_back({static_cast<std::string_view>(nodes_strings_).
                   substr(begin, input_buffer_.size()),
                   0, 0,
                   tree_.size() - 1});
}
