#include <iostream>
#include <chrono>
#include <thread>

#include "tests/ConnectionTests.hpp"

int main(int, const char *[]) {
  std::cout << std::endl;
  std::cout << "========== Executing Crier Tests ==========" << std::endl;
  std::cout << " > Connection Tests: " << (TestCrierTransportConnection() ? "PASSED" : "FAILED") << std::endl;
  std::cout << std::endl;
}
