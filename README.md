# Crier - An event system tailored for protobuffer based network communication

Crier is a thread-safe, header-only, protobuf based, event dispatching system designed to cut the cruft in managing asynchronous communications and protocol buffer object serialization.
Using protocol buffer reflections it allows you to easily manage your communications, enabling you to only treat the right messages in the right part of your application, not having to worry about writting code to identify which message arrived, and passing it across your application.

## Crier allows you to:

- Define a transport class, which you can use to attach crier to any kind of communication socket you want to use the system with.
- Send Proto Buffer objects through crier, with the option to expect an object in response and the ability to define a timeout behaviour if the response doesn't arrive.
- Register callbacks to always listen to specific objects coming over from the transport.
- Dispatch callbacks the moment the object arrives on the transport (in whatever thread it might be), or queue for release upon demand (on an update loop, for example).
- Define behaviours for messages with no callback assigned.
- If your callbacks for specific objects are order sensitive, mark them as Priority to be handled first, ASAP to be handled after, or Normal to be handled at the end.

## Requirements

- C++11 compliant compiler
- Google Protocol Buffers (including the reflection library)

## How to Use

The crier lib is based on the usage of it's main template class Crier. To use the lib effectively, all you have to do is create an instance and use its methods.

# To create a Crier instance you need:
- A protobuf definition file that defines a root message which contains any other message you want to send or receive, as in this example:
```
package example_proto;

/// Example message
message test1 {
    required uint32 id = 1;
}

/// Example message
message test2 {
    required string name = 1;
}

/// Root message required for Crier to function
/// Should be a message with optional fields for each other message defined in the protobuf file
message root_msg { 
    optional test1 test1_field = 1;              
    optional test2 test2_field = 2;
}
```

- An implementation of the Transport API (found in Transport.hpp)
To allow for freedom in how your application opens connections and sends data (tcp, udp, websocket, etc...) a class implementing the Transport API is required. 
It should essentially follow a decorator pattern over the transport you want to use. 
You can then pass an initialized instance of this Transport over to crier upon initialization, or you can allow crier to allocate a default instance of your transport. In any of these cases, crier will have ownership of the instance.

Both of these elements are what constitutes the templated parts of the crier class
```C++
Crier<TransportClass, example_proto::root_msg>
```

# Connecting

To connect to a service through a crier instance, first you need to instance it. Follow the constructor documentation if you need anything other than a default crier.
Then, to connect, simply call:
```C++
crier_instance.ConnectTransport(ip_address, port);
```

# Common Usage Examples

- Send a Message:
```C++
example_proto::test1 message;
message.set_id(1);

crier_instance.sendMessage(message);
```

- Send a Message, while registering a single use callback for a response
```C++
example_proto::test1 message;
message.set_id(1);

crier_instance.sendMessageWithRetCallback<example_proto::test1, example_proto::test2>(message, 
    [](const example_proto::test2& response_msg){
        /// Do stuff to treat the response
        /// This will be called only if a response of this type (test2) arrives
    });
```

- Send a Message, while registering a single use callback for a response, but with a timeout period associated to another callback
```C++
example_proto::test1 message;
message.set_id(1);

crier_instance.sendMessageWithRetCallbackAndTimeout<example_proto::test1, example_proto::test2>(message, 
    [](const example_proto::test2& response_msg){
        /// Do stuff to treat the response
        /// This will be called only if a response of this type (test2) arrives
    },
    2000, /// Time for the response to arrive, in milliseconds
    [](){
        /// This will be called if 2000 milliseconds pass without a return of type test2 arrives
        /// If this callback is called, the above reponse callback will be un-registered and never called (even if a response arrives later)
    });
```

- Register a permanent callback for a received message, not associated with any outbound message
```C++
/// UNIQUE_ID is required to identify this callback for later removal. If the exact same id is used to register a callback to the exact same message, the previous callback will be overwritten
crier_instance.registerPermanentCallback<example_proto::test2>("<UNIQUE_ID>", 
    [](const example_proto::test2& msg) {
        /// This callback will be called every time a test2 message arrives
    });

crier_instance.clearPermanentCallback<example_proto::test2>("<UNIQUE_ID>"); /// UNIQUE_ID is required for removal of the callback
```

For more information on all of Crier's capabilities, check the header Crier.hpp for the full API documentation

## Author

- Pedro Engana (Enagan) <pedro.a.f.engana@gmail.com>

## License

Crier is available under the MIT license (see `LICENSE`).