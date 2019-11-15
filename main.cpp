#include <iostream>
#include <Akinator.h>
#include <string.h>

int main(int argc, char** argv) {
  Akinator akinator(argc == 1 ? "" : argv[1]);
  akinator.InteractiveMode();
  return 0;
}