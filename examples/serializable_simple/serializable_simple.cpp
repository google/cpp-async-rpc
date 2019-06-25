/// \file
/// \brief Explicit struct serialization demo.
///
/// \copyright
///   Copyright 2019 by Google LLC.
///
/// \copyright
///   Licensed under the Apache License, Version 2.0 (the "License"); you may
///   not use this file except in compliance with the License. You may obtain a
///   copy of the License at
///
/// \copyright
///   http://www.apache.org/licenses/LICENSE-2.0
///
/// \copyright
///   Unless required by applicable law or agreed to in writing, software
///   distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
///   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
///   License for the specific language governing permissions and limitations
///   under the License.

#include <iostream>
#include <string>
#include <vector>
#include "arpc/binary_codecs.h"
#include "arpc/iostream_adapters.h"
#include "arpc/serializable.h"

// Simple structs that have no base classes and can be constructed via aggregate
// initialization are "reflected" automatically and serialized as if they were
// tuples of the same types as their fields' types.
struct MyClass {
  int x;
  double y;
  std::vector<std::string> z;
};

int main(int argc, char* argv[]) {
  // Create an instance of my data structure.
  MyClass data{4, 5.5, {"first", "second", "third"}};

  // Create a binary encoder outputting to stdout.
  arpc::ostream_output_stream oos(std::cout);
  arpc::little_endian_binary_encoder encoder(oos);

  // Write the binary data. Try piping the output of this program to xxd.
  encoder(data);

  return 0;
}
