#include <Akinator.h>
#include <cstring>
#include <cassert>

struct PropertyNode {
    size_t index;
    bool property;
};

Akinator::Akinator(const std::string& filename) {
  tree_.ReadTree(filename);
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
      ShowTreeMode();
    } else if (input_buffer_ == "exit") {
      return;
    }
  }
}

void Akinator::TraversalMode() {
  size_t current_node = 0;
  while (true) {
    if (!tree_.IsLeaf(current_node)) {
      std::printf("%.*s? (y or n):",
        tree_.GetTree().at(current_node).string.size(),
        tree_.GetTree().at(current_node).string.data());
      ReadLine();
      if (input_buffer_ == "y") {
        current_node = tree_.GetTree().at(current_node).left;
      } else if (input_buffer_ == "n") {
        current_node = tree_.GetTree().at(current_node).right;
      } else {
        std::printf("Answer y or n");
      }
      continue;
    }

    std::printf("Is it %.*s? (y or n):",
      tree_.GetTree().at(current_node).string.size(),
      tree_.GetTree().at(current_node).string.data());
    ReadLine();
    if (input_buffer_ == "y") {
      return;
    } else if (input_buffer_ == "n") {
      std::printf("Please, write its correct attribute:");
      ReadLine();
      std::string property(input_buffer_);
      std::printf("Please, write your character:");
      ReadLine();
      tree_.UpdateTree(current_node, property, input_buffer_);
      return;
    } else {
      std::printf("Answer y or n\n");
      continue;
    }
  }
}

void Akinator::DifferenceMode() const {
  auto find_node = [this]() {
    auto it = ReadAndFindCharacter();
    if (it == tree_.GetTree().end())
      std::printf("there is no this character\n");
    return it;
  };

  std::printf("Enter first character:");
  auto first_node_it = find_node();
  if (first_node_it == tree_.GetTree().cend()) return;
  size_t first_node = std::distance(tree_.GetTree().cbegin(), first_node_it);

  std::printf("Enter second character:");
  auto second_node_it = find_node();
  if (second_node_it == tree_.GetTree().cend()) return;
  size_t second_node = std::distance(tree_.GetTree().cbegin(), second_node_it);

  size_t lca = tree_.LCA(first_node, second_node);

  std::vector<PropertyNode> properties_stack;
  properties_stack.reserve(tree_.GetTree().size() / 2 + 1);

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

void Akinator::WriteTreeMode() const {
  std::printf("Write filename for tree:");
  ReadLine();
  tree_.WriteTree(input_buffer_);
}

void Akinator::ShowTreeMode() const {
  std::printf("Write filename for tree(*.png preferred):");
  ReadLine();
  input_buffer_ += ".dot";
  FILE* dot_file = fopen((input_buffer_).c_str(), "w");
  std::fprintf(dot_file,
    "digraph akitree {\n"
    "graph [nodesep=0.5];\n"
    "node [fontname=\"Helvetica\", color=\"Lavender\", style=\"filled\"];\n");
  for (size_t i = 0; i < tree_.GetTree().size(); ++i) {
    std::fprintf(dot_file,
      "node%zu [label=\"%.*s\"];\n", i,
      tree_.GetTree().at(i).string.size(),tree_.GetTree().at(i).string.data());
  }

  for (size_t i = 0; i < tree_.GetTree().size(); ++i) {
    if (!tree_.IsLeaf(i)) {
      std::fprintf(dot_file,
        "edge [color=\"Green\"];\n"
        "node%zu -> node%zu;\n",
        i, tree_.GetTree().at(i).left);
      std::fprintf(dot_file,
        "edge [color=\"Red\"];\n"
        "node%zu -> node%zu;\n",
        i, tree_.GetTree().at(i).right);
    }
  }
  std::fwrite("}\n", 2, 1, dot_file);
  fclose(dot_file);

  input_buffer_.resize(input_buffer_.size() - 4);
  std::string formatted_str(strlen("dot  -Tpng -o ") +
                            input_buffer_.size() * 2 + 4, 0);
  std::sprintf(formatted_str.data(),
               "dot %.*s.dot -Tpng -o %.*s",
               input_buffer_.size(), input_buffer_.c_str(),
               input_buffer_.size(), input_buffer_.c_str());
  system(formatted_str.c_str());
  formatted_str.resize(input_buffer_.size() + strlen("pix  &> /dev/null"));
  std::sprintf(formatted_str.data(),
               "pix %.*s &> /dev/null",
               input_buffer_.size(),
               input_buffer_.c_str());
  system(formatted_str.c_str());
}

void Akinator::ReadLine() const {
  std::fflush(stdout);
  while (std::isspace(std::cin.peek())) std::cin.get();
  std::getline(std::cin, input_buffer_);
}

std::vector<AkinatorTree::Node>::const_iterator
Akinator::ReadAndFindCharacter() const {
  ReadLine();
  for (size_t i = 0; i < tree_.GetString().size(); ++i) {
    if (std::string_view(input_buffer_) == tree_.GetTree().at(i).string &&
        tree_.IsLeaf(i)) {
      return  tree_.GetTree().cbegin() + i;
    }
  }
  return tree_.GetTree().cend();
}

void Akinator::CollectProperties(size_t from, size_t to,
                                std::vector<PropertyNode>& stack) const {
  auto is_true_property = [this, &stack](size_t index) {
    return tree_.GetTree().at(tree_.GetTree().at(index).parent).left == index;
  };

  stack.resize(0);
  if (from != to)
    stack.push_back({tree_.GetTree().at(from).parent, is_true_property(from)});
  while (stack.back().index != to) {
    stack.push_back({tree_.GetTree().at(stack.back().index).parent,
                     is_true_property(stack.back().index)});
  }
}

void
Akinator::PrintProperties(std::vector<PropertyNode>& stack) const {
  while (!stack.empty()) {
    if (!stack.back().property)
      std::printf("NOT ");
    assert(!tree_.GetTree().at(stack.back().index).string.empty());
    std::printf("%.*s; ",
      tree_.GetTree().at(stack.back().index).string.size() - 1,
      tree_.GetTree().at(stack.back().index).string.data());
    stack.pop_back();
  }
  std::putchar('\n');
  std::fflush(stdout);
}
