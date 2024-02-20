#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
  std::cout << argc << std::endl;
  std::cout << argv[0] << std::endl;
  std::cout << argv[1] << std::endl;
  std::cout << argv[2] << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  return 0;
}