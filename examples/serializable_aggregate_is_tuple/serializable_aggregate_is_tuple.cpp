/// \file
/// \brief Simple struct serialization demo.
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
#include <tuple>
#include <vector>
#include "arpc/binary_codecs.h"
#include "arpc/serializable.h"
#include "arpc/string_adapters.h"

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

  // Create a binary encoder outputting to a string.
  std::string buf;
  arpc::string_output_stream sos(buf);
  arpc::little_endian_binary_encoder encoder(sos);

  // Write the data, prefixing it with a structure verification tag.
  encoder(data, arpc::verify_structure{});

  // Create a binary decoder reading from the string.
  arpc::string_input_stream sis(buf);
  arpc::little_endian_binary_decoder decoder(sis);

  // Now read the data as an equivalent tuple; check the structure verification
  // tag for compatibility first.
  std::tuple<int, double, std::vector<std::string>> read_data;
  decoder(read_data, arpc::verify_structure{});

  // Output the result.
  std::cout << std::get<0>(read_data) << ", " << std::get<1>(read_data)
            << ", {";
  for (const auto& v : std::get<2>(read_data)) {
    std::cout << "\"" << v << "\", ";
  }
  std::cout << "}" << std::endl;

  return 0;
}
