#include <AkinatorTreeFuncs.h>
#include <headers/AkinatorTreeFuncs.h>

void DotFileFromAkinatorTree(const AkinatorTree& tree,
                             const std::string& filename) {
  FILE* dot_file = fopen((filename).c_str(), "w");
  std::fprintf(dot_file,
    "digraph akitree {\n"
    "graph [nodesep=0.5];\n"
    "node [fontname=\"Helvetica\", color=\"Lavender\", style=\"filled\"];\n");
  for (size_t i = 0; i < tree.GetTree().size(); ++i) {
    std::fprintf(dot_file,
      "node%zu [label=\"%.*s\"];\n", i,
      tree.GetTree().at(i).string.size(),tree.GetTree().at(i).string.data());
  }

  for (size_t i = 0; i < tree.GetTree().size(); ++i) {
    if (!tree.IsLeaf(i)) {
      std::fprintf(dot_file,
        "edge [color=\"Green\"];\n"
        "node%zu -> node%zu;\n",
        i, tree.GetTree().at(i).left);
      std::fprintf(dot_file,
        "edge [color=\"Red\"];\n"
        "node%zu -> node%zu;\n",
        i, tree.GetTree().at(i).right);
    }
  }
  std::fwrite("}\n", 2, 1, dot_file);
  fclose(dot_file);
}
