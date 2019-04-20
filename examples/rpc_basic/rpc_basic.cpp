///

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

  // Set up and the server on port 9999 and register the "greeter" object in it.
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
