#pragma once
#include <AkinatorTree.h>
#include <AkinatorTreeFuncs.h>
#include <cstdio>
#include <iostream>

struct PropertyNode;

class Akinator {
 public:
  Akinator() = delete;
  Akinator(const std::string& filename);
  void InteractiveMode();
  void TraversalMode();
  void DifferenceMode() const;
  void WriteTreeMode() const;
  void ShowTreeMode() const;
 private:
  void ReadLine() const;
  std::vector<AkinatorTree::Node>::const_iterator ReadAndFindCharacter() const;
  void CollectProperties(size_t from, size_t to,
                         std::vector<PropertyNode>& stack) const;
  void PrintProperties(std::vector<PropertyNode>& stack) const;
  AkinatorTree tree_;
  mutable std::string input_buffer_;
};