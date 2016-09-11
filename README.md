# Crier - An event system tailored for protobuffer based network communication

Crier is a thread-safe, header-only, protobuff based, event dispatching system designed to cut the cruft in managing asynchronous communications and protocol buffer object serialization.
Using protocol buffer reflections it allows you to easily manage your communications, enabling you to only treat the right messages in the right part of your application, not having to worry about writting code to identify which message arrived, and passing it on across your application.

## Crier allows you to:

- Define a Crier transport class, which you can use to attach it to any kind of communication socket you want to use the system with.
- Send Proto Buffer objects through said transport, with the option to expect an object in response, and the ability to define a timeout behaviour if the response doesn't arrive.
- Register callbacks to always listen to specific objects coming over from the transport.
- Dispatch callbacks the moment the object arrives on the transport (in whatever thread it might be), or queue for release upon demand (on an update loop, for example).
- Define behaviours for messages with no callback assigned.
- If your callbacks for specific objects are order sensitive, mark them as Priority to be handled first, ASAP to be handled after, or Normal to be handled at the end.

## Requirements

- C++11 compliant compiler
- Google Protocol Buffers (including the reflection library)

## Usage Examples

- Coming Soon!

## Author

- Pedro Engana (Enagan) <pedro.a.f.engana@gmail.com>

## License

Crier is available under the MIT license (see `LICENSE`).