/// \file
/// \brief Basic test of same-process RPC server and client.
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
#include "lasr/client.h"
#include "lasr/interface.h"
#include "lasr/server.h"

/// This is the definition of the Greeter interface.
LASR_INTERFACE(Greeter, (/* doesn't extend other interfaces */),
               (  // Return a salutation..
                   ((std::string), say_hello_to,
                    (((const std::string&), name)))));

/// This is the server-side implementation of the Greeter interface.
struct GreeterImpl : Greeter {
  std::string say_hello_to(const std::string& name) override {
    return "Hello " + name + "!";
  }
};

int main(int argc, char* argv[]) {
  // Create a server-side implementation object.
  lasr::server_object<GreeterImpl> greeter;

  // Set up the server on TCP port 9999 and register the object in it with under
  // the "greeter" name.
  lasr::server server({/* default options */}, lasr::endpoint().port(9999));
  server.register_object("greeter", greeter);

  // Start serving RPCs.
  server.start();

  // Create a client connection to the server.
  lasr::client_connection client(lasr::endpoint().name("localhost").port(9999));

  // Get a proxy for the "greeter" remote object.
  auto greeter_proxy = client.get_proxy<Greeter>("greeter");

  // Call the remote method and print the results.
  std::cout << greeter_proxy.say_hello_to("world") << std::endl;

  return 0;
}
