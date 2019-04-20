# lasr: Library for Asynchronicity, Serialization and Remoting

This is lasr, a C++17 library supporting template meta-programming, asynchronous
network programming, binary serialization and RPC.

## What does lasr look like?

```c++
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
```

## Why lasr?

I wanted to communicate two microcontrollers without having to write the code
to actually put the data on the serial line. Eventually the project grew up to
be a fully fledged serialization library, using template meta-programming
techniques to simplify usage and resolve as much as possible at compile time.

At that point, building on top of the serialization capabilities to create a
fully fledged RPC service became tempting, so I needed the networking support...
and of course it would be bad to use a "thread per connection" model in an
embedded environment, so we needed some asynchronous mechanisms too (there's
some tuning for running in embedded systems).

Albeit lasr originally targeted C++11, eventually the fixes in later versions
made everything simpler and more powerful, thus justifying moving to C++17.

At this point the library supports POSIX environments (it's developed on Linux),
but the plan is to eventually make it work for Espressif's
[ESP-IDF](https://github.com/espressif/esp-idf1) environment, so that RPC
servers can be implemented in the
[ESP32](https://www.espressif.com/en/products/hardware/esp32/overview) hardware
platform.

## What's in lasr?

 * Preprocessor-based code generation (sequences, iteration and conditionals).
   This is used so that lasr doesn't require an IDL compiler to define its
   serializable structures or RPC interfaces. Thanks to the preprocessor, these
   are just defined in C++ native headers (albeit with some syntax contortions).

 * Meta-programming toolkit: support for compile-time sequences with associated
   compile-time or run-time values. Support compile-time iteration on list of
   types or values. Compile-time and `constexpr` mapping and aggregation of
   data sequences. 

 * A serialization framework supporting portable binary serialization of most of
   the standard types in modern C++, including primitive types, strings,
   sequences and associative containers. Smart pointers (including run-time
   polymorphism and object graphs) are also supported.

 * Asynchronous primitives for operating on files, sockets and timers, coupled
   with synchronization primitives (mutexes, semaphores, flags, queues, futures)
   that also support asynchronicity and event composition (via `select`). The
   synchronization primitives are drop-in replacements for the standard library
   ones, but they are built on POSIX pipes so that they can be used in `select`
   or `poll`.

 * A superset of `std::thread` with support for stackable execution contexts.
   The `context` primitive can hold data or deadlines to be propagated across
   RPC call boundaries. The contexts can also be cancelled triggering exceptions
   whenever the associated thread attempts any asynchronous or I/O operation.

 * Networking support for resolving domain names and ports, and establishing
   both client and server connections.

 * Preprocessor-based generation of RPC-oriented "C++ interfaces" (classes with
   just pure virtual methods), including asynchronous variants and the
   associated RPC proxy implementations.
   
 * RPC client and server support for remoting over RPC.

## What's still missing?

 * Bug fixes (at this point it's alpha quality and unstable).

 * An actual port to the ESP32 (which will likely involve some
   [VFS](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/vfs.html).
   component to support vestigial pipes as select-able locks.
 
 * Non-token test coverage.

 * Documentation.
 
## Inspiration

 * [cereal](http://uscilab.github.io/cereal/): A C++11 library for
   serialization. This was the original source of inspiration for lasr.
   
 * [Boost](https://www.boost.org/) MPL, Fusion and Hana libraries. These
   provided inspiration on how to process sequences of types or data at compile
   time (for example for iterating on struct fields).
   
 * [The Go language](https://golang.org/). While marred by some early design
   decissions (the lack of generics and exceptions, in particular), other areas
   of the language like the event multiplexing primitives or the design of its
   networking libraries really do shine.
 
## Dependencies
 
 * A [C++17](https://en.wikipedia.org/wiki/C%2B%2B17) compliant compiler.

 * [The Meson Build system](https://mesonbuild.com/).
 
 * [Catch2](https://github.com/catchorg/Catch2) for the unit testing framework.

 * [fu2::function2](https://github.com/Naios/function2) as a handy replacement
   for `std::function` that can support move-only lambdas too. 