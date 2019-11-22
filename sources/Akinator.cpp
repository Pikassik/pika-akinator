#include <Akinator.h>
#include <cstring>
#include <cassert>
#include <cstdlib>

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
  std::vector<size_t> not_stated_stack;
  not_stated_stack.reserve(tree_.GetTree().size() / 2 + 1);
  size_t current_node = 0;
  while (true) {
    if (!tree_.IsLeaf(current_node)) {
      PrintFormatted("%.*s:", tree_.NodeToString(current_node));
      ReadLine();

      if (input_buffer_ == "idk") {
        input_buffer_ = rand() % 2 ? "ns y" : "ns n";
      }

      if (input_buffer_ == "y") {
        current_node = tree_.GetTree().at(current_node).left;
      } else if (input_buffer_ == "n") {
        current_node = tree_.GetTree().at(current_node).right;
      } else if (input_buffer_ == "ns y") {
        not_stated_stack.push_back(tree_.GetTree().at(current_node).right);
        current_node = tree_.GetTree().at(current_node).left;
      } else if (input_buffer_ == "ns n") {
        not_stated_stack.push_back(tree_.GetTree().at(current_node).left);
        current_node = tree_.GetTree().at(current_node).right;
      }  else {
        printf("Answer y (yes) or n (no) or "
               "ns y (not sure yes) or ns n (not sure no)\n");
      }
      continue;
    }

    PrintFormatted("Is it %.*s?:", tree_.NodeToString(current_node));
    ReadLine();

    if (input_buffer_ == "y")
      return;

    if (input_buffer_ == "n") {
      if (!not_stated_stack.empty()) {
        current_node = not_stated_stack.back();
        not_stated_stack.pop_back();
        continue;
      }
      std::printf("Please, write its correct attribute:");
      ReadLine();
      std::string property(input_buffer_);
      std::printf("Please, write your character:");
      ReadLine();
      tree_.UpdateTree(current_node, property, input_buffer_);
      return;
    }

    std::printf("Answer y or n\n");
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
  PrintString("Common properties: ");
  PrintProperties(properties_stack);

  CollectProperties(first_node, lca, properties_stack);
  PrintFormatted("%.*s's unique properties: ",
                 tree_.NodeToString(first_node));
  PrintProperties(properties_stack);

  CollectProperties(second_node, lca, properties_stack);
  PrintFormatted("%.*s's unique properties: ",
                 tree_.NodeToString(second_node));
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
  DotFileFromAkinatorTree(tree_, input_buffer_);

  input_buffer_.resize(input_buffer_.size() - 4);
  std::string formatted_str(strlen("dot  -Tpng -o  &> /dev/null") +
                            input_buffer_.size() * 2 + 4, 0);
  std::sprintf(formatted_str.data(),
               "dot %.*s.dot -Tpng -o %.*s &> /dev/null",
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

void Akinator::PrintFormatted(const char* format,
                              const std::string_view& string) const {
    printf(format, string.size(), string.data());
    std::fflush(stdout);
    std::string buffer_fin(strlen("echo \"\" | espeak -s 80") +
                           strlen(format) + string.size(), 0);
    std::string buffer_not_fin(strlen(format) + string.size(), 0);

    std::sprintf(buffer_not_fin.data(), format,
                 string.size(), string.data());
    std::sprintf(buffer_fin.data(), "echo \"%.*s\" | espeak -s 80",
                 buffer_not_fin.size() - 4, buffer_not_fin.c_str());
    system(buffer_fin.data());
}

void Akinator::PrintString(const std::string_view& string) const {
    printf("%.*s", string.size(), string.data());
    std::fflush(stdout);
    std::string buffer_fin(strlen("echo \"\" | espeak -s 120") +
                                  string.size(), 0);
    std::sprintf(buffer_fin.data(), "echo \"%.*s\" | espeak -s 120",
                 string.size(), string.data());
    system(buffer_fin.c_str());

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
    if (std::string_view(input_buffer_) == tree_.NodeToString(i) &&
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
  input_buffer_.clear();
  while (!stack.empty()) {
    if (!stack.back().property)
      input_buffer_ += "NOT ";
    assert(!tree_.NodeToString(stack.back().index).empty());

    input_buffer_ += std::string_view(tree_.NodeToString(stack.back().index)).
                   substr(0, tree_.NodeToString(stack.back().index).size() - 1);
    input_buffer_.push_back(' ');
    stack.pop_back();
  }
  PrintString(input_buffer_);
  std::putchar('\n');
  std::fflush(stdout);
}
