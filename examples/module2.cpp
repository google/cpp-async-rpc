#include "module2.h"
#include <chrono>
#include <iostream>
#include "ash/io.h"

void run_module2() {
  ash::channel in(0);
  auto s =
      ash::select(in.read(), ash::timeout(std::chrono::milliseconds(3000)));
  std::cerr << s[0] << s[1] << std::endl;
  in.release();
}
