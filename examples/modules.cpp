#include <chrono>
#include <iostream>
#include "ash/io.h"

#include "module1.h"
#include "module2.h"

int main() {
  ash::channel in(0);
  auto s =
      ash::select(in.read(), ash::timeout(std::chrono::milliseconds(3000)));
  std::cerr << s[0] << s[1] << std::endl;
  in.release();

  run_module1();
  run_module2();

  return 0;
}
