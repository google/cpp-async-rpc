# cpp-async-rpc: Library for Asynchronicity, Serialization and Remoting

This is cpp-async-rpc, a C++17 library supporting template meta-programming,
asynchronous network programming, binary serialization and RPC.

**Disclaimer:** This is not a Google supported product.

## What does cpp-async-rpc look like?

### RPC server and client

Here a single small binary includes an interface definition, the server and the
client.

```c++
#include <iostream>
#include <string>
#include "arpc/client.h"
#include "arpc/interface.h"
#include "arpc/server.h"

/// This is the definition of the Greeter interface.
ARPC_INTERFACE(Greeter, (/* doesn't extend other interfaces */),
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
  arpc::server_object<GreeterImpl> greeter;

  // Set up the server on TCP port 9999 and register the object in it with under
  // the "greeter" name.
  arpc::server server({/* default options */}, arpc::endpoint().port(9999));
  server.register_object("greeter", greeter);

  // Start serving RPCs.
  server.start();

  // Create a client connection to the server.
  arpc::client_connection client(arpc::endpoint().name("localhost").port(9999));

  // Get a proxy for the "greeter" remote object.
  auto greeter_proxy = client.get_proxy<Greeter>("greeter");

  // Call the remote method and print the results.
  std::cout << greeter_proxy.say_hello_to("world") << std::endl;

  return 0;
}
```

Yeah, we use the C preprocessor as our IDL compiler, which makes the syntax
awkward, but on the other hand your interface definition remains valid (even if
ugly) C++ header source. 

### Serialization of custom structures

While cpp-async-rpc is able to serialize and deserialize most standard library
classes and others implementing compatible interfaces out of the box, preparing
your own data structures for serialization (or their use as RPC arguments) is
easy:

```c++
#include <iostream>
#include <string>
#include <vector>
#include "arpc/binary_codecs.h"
#include "arpc/iostream_adapters.h"
#include "arpc/serializable.h"

// To be serializable, MyClass must inherit arpc::serializable<MyClass> or
// arpc::dynamic<MyClass> if run-time polymorphism is required.
struct MyClass : arpc::serializable<MyClass> {
  int x;
  double y;
  std::vector<std::string> z;

  // Enumerate the fields that must be serialized.
  ARPC_FIELDS(x, y, z);
};

int main(int argc, char* argv[]) {
  // Create an instance of my data structure.
  MyClass data{{/* base class init */}, 4, 5.5, {"first", "second", "third"}};

  // Create a binary encoder outputting to stdout.
  arpc::ostream_output_stream oos(std::cout);
  arpc::little_endian_binary_encoder encoder(oos);

  // Write the binary data. Try piping the output of this program to xxd.
  encoder(data);

  return 0;
}
```

### Asynchronous networking

This examples sends an HTTP get request and prints out the response in an
asynchronous manner, and using a local context to set a timeout of 10 seconds
for the whole set of calls.

```c++
#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include "arpc/awaitable.h"
#include "arpc/context.h"
#include "arpc/errors.h"
#include "arpc/select.h"
#include "arpc/socket.h"

int main(int argc, char* argv[]) {
  try {
    arpc::context ctx;
    ctx.set_timeout(std::chrono::seconds(10));

    auto s =
        arpc::dial(arpc::endpoint().name("www.kernel.org").service("http"));

    std::string request = "GET / HTTP/1.0\r\nHost: www.kernel.org\r\n\r\n";
    char buf[256];

    while (true) {
      auto [sent, received] = arpc::select(
          request.size() ? s.async_write(request.data(), request.size())
                         : arpc::never().then([]() { return std::size_t{0}; }),
          s.async_read(buf, sizeof(buf)));

      if (sent) {
        std::cout << "S(" << *sent << ")" << std::endl;
        // Remove the bytes we already sent.
        request.erase(0, *sent);
      }

      if (received) {
        std::cout << "R(" << *received << ")" << std::endl
                  << std::string(buf, buf + *received) << std::endl;
      }
    }

    return 0;
  } catch (const arpc::errors::base_error& e) {
    std::cerr << "Exception of type " << e.portable_error_class_name()
              << " with message: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Some other exception." << std::endl;
    return 1;
  }
}
```

## Why cpp-async-rpc?

This library evolved from plans to communicate two distinct microcontrollers
over a serial line.

It eventually became a generic serialization framework (because none of the
existing ones would be easy to adapt to the target embedded environments), and
from there on the jump to an RPC framework with support with cancellation and
asynchronous networking was easier.    

As cpp-async-rpc built on template metaprogramming to reduce the amount of code
the user would need to write, the library eventually moved from targeting C++11
to target C++17, which simplified a number of things and made some syntax way
more natural.

At of today, the library supports POSIX environments (it's developed on Linux),
but the plan is to eventually make it work for Espressif's
[ESP-IDF](https://github.com/espressif/esp-idf1) environment, so that RPC
servers can be implemented in the
[ESP32](https://www.espressif.com/en/products/hardware/esp32/overview) hardware
platform.

## What's in cpp-async-rpc?

 * Preprocessor-based code generation (sequences, iteration and conditionals).
   This is used so that arpc doesn't require an IDL compiler to define its
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

 * Discovering and fixing all the bugs.

 * A usable port to the ESP32 (which will likely involve some
   [VFS](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/vfs.html).
   component to support vestigial pipes as select-able locks.
 
 * Non-token test coverage.

 * Documentation.
 
 * Implement missing features:
 
   * "catch" support for `awaitable` and `future` deferred behaviour.
 
## Inspiration

 * [Cereal](http://uscilab.github.io/cereal/): A C++11 library for
   serialization. This was the original source of inspiration for arpc.
   
 * [Boost](https://www.boost.org/) MPL, Fusion and Hana libraries. These
   provided inspiration on how to process sequences of types or data at compile
   time (for example for iterating on struct fields).
   
 * [The Go language](https://golang.org/). The cancellable contexts, the channel
   multiplexing and the clean networking libraries have inspired some of the
   work in arpc.
   
 * [JavaScript Promises](https://developer.mozilla.org/es/docs/Web/JavaScript/Referencia/Objetos_globales/Promise)
   as the chosen way to encapsulate behaviour to be run once asynchronous
   conditions are met.
 
## Dependencies
 
 * A [C++17](https://en.wikipedia.org/wiki/C%2B%2B17) compliant compiler.

 * [The Meson Build system](https://mesonbuild.com/).
 
 * [Catch2](https://github.com/catchorg/Catch2) for the unit testing framework.

 * [fu2::function2](https://github.com/Naios/function2) as a handy replacement
   for `std::function` that can support move-only lambdas too. 
