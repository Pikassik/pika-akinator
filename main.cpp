#include <iostream>
#include <Akinator.h>
#include <string.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "Please, enter filename";
    return 0;
  }
  Akinator akinator;
  akinator.ReadFile("../input");
  akinator.InteractiveMode();
//  akinator.WriteFile("../out");
  return 0;
}