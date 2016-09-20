#ifndef CRIER_HPP
#define CRIER_HPP

#include <memory>
#include <map>
#include <deque>
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <forward_list>
#include <utility>
#include <atomic>
#include <list>
#include <typeinfo>
#include <typeindex>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <crier/CrierTypes.hpp>

namespace crier {

  /// Crier API
  /// Main Class for using crier. As per it's template parameters it will take a Transport class and the generated class for the .proto root message.
  /// - the Transport must be a class which follows the API defined in the helper concept hpp, 'TransportConcept.hpp'. The purpose of this class is to allow the API user to
  ///    write it as a decorator pattern over whichever kind of transport he wants to use to communicate (tcp, udp, websocket, outstream, etc...). 
  ///    Crier will take ownership (or create it's own) instance of this Transport, and use it as it's underlying inbound/outbound.
  /// - the ProtoRootMsg must be the generated protocol buffer class that represents the root message of the protocol you wish to use with this crier instance. 
  ///    If this sounds very confusing, please check the 'How to Use' of the Readme at the root of this repo. Crier will use this message in reflection in order to figure out
  ///    which message it's sending, or which message it has just received.
  /// Multiple crier instances can be created, using different transports and even different protocols
  template <typename Transport, typename ProtoRootMsg>
  class Crier {
  public:

    /// Basic constructor for a crier instance. Creating a crier instance has two optional parameters that affect the default behaviour for all incoming messages.
    /// This default behaviour can be changed later through the appropriate methods, or message specific behaviours can also be set.
    /// - default_unhandled_behaviour, specifies what crier should do when receiving a message that has 0 registered callbacks to listen to it. By default, the option is ignore
    ///   which will simply discard the message, the other option in enqueue, which will save the message to be dispatched when the first permanent callback for it is registered.
    /// - default_inbound_dispatch, specifies how crier should invoke the callbacks when messages arrive, the default is 'Immediate', which will call the lambdas as soon as messages arrive 
    ///   through the transport. This means, for example, if your transport inbound runs on a separate thread, your callbacks will run on that thread (crier is thread-safe, but the 
    ///   side-effects of your callbacks might not be, so take care). The other option is 'DispatchQueue', which will save the callbacks in a queue which is only dispatched when (and where) 
    ///   you call the 'dispatchQueuedCallbacks' method.
    /// When using this constructor, keep in mind that crier will create it's own instance of the Transport class it was templated with, which means that class must have a default constructor.
    /// If having a default constructor is impossible or impractical for whatever reason, check the other constructor below.
    Crier(UnhandledMessageBehaviour default_unhandled_behaviour = UnhandledMessageBehaviour::Ignore,
          InboundDispatching default_inbound_dispatch = InboundDispatching::Immediate);

    /// Consructor with transport copy/move. 
    /// If your implementation of the Transport concept needs to be initialized with extra parameters before giving up it's control to the crier instance, then you can use this constructor.
    /// In this use case, you have to ensure your transport class has a valid copy constructor (which hopefully doesn't break your initializations). Alternatively, since the implementation
    /// uses the copy-and-swap idiom, you can just define a move constructor for your transport and move your instance into the constructor, for better performance (or aversion to copies).
    Crier(Transport transport, UnhandledMessageBehaviour default_unhandled_behaviour = UnhandledMessageBehaviour::Ignore,
          InboundDispatching default_inbound_dispatch = InboundDispatching::Immediate);
    
    /// Crier instances cannot be copied due to their nature (it doesn't make sense to copy an open connection, what would it even mean?) 
    Crier(const Crier& copy) = delete;

    /// Crier instances cannot be moved due to their internal implementation (since it uses mutexes for multi-thread safety, and these are not trivially moveable) 
    Crier(Crier&& copy) = delete;

    void operator=(const Crier& copy) = delete;
    void operator=(Crier&& copy) = delete;
    
    ~Crier();
    
// -- Transport Handling
// Methods to directly deal with the transport instance owned by crier

    /// Connects the transport to the given ip and port (will call the void 'connect(const std::string& host, int ip)' on your Transport concept implementation)
    void ConnectTransport(const std::string& ip, unsigned int port);

    /// Disconnects the transport from whichever active connection it currently has (will call the void 'void disconnect()' on your Transport concept implementation)
    void DisconnectTransport();

    /// Returns true if the Transport has an active connection, false otherwise (will call the void 'bool isConnected()' on your Transport concept implementation)
    bool TransportConnected() const;

    /// Fetches a reference to the crier managed transport instance. Typically you shouldn't mess with it during regular usage, but it's available for edge cases
    Transport& transport();
    
// -- Message Sends
// Methods to send protobuf messages through the transport

    /// Sends a message of type MsgData.
    /// The MsgData class must be present in your generated protocol buffer .cc and .h, and it must be contained in your root message (ProtoRootMsg template)
    /// as an optional member (or through a protobuf extension, check readme for an example of how your root message should look like).
    /// If these requirements are not met, crier won't be able to bundle the message and won't do anything.
    template <typename MsgData>
    void sendMessage(const MsgData& data);
    
    /// Sends a message of type MsgData, and registers a callback, expecting a response of type RetMsgData
    /// Both the MsgData and RetMsgData classes must be present in your generated protocol buffer .cc and .h, and they must be contained in your root message (ProtoRootMsg template)
    /// as optional members (or through protobuf extension, check readme for an example of how your root message should look like).
    /// - data, the object of type ReqMsgData that you want to send
    /// - onSuccess, callback receiving a RetMsgData as a response. Keep in mind that this is seen as a "consumable" callback for crier. This means the first message to arrive of
    ///   RetMsgData type will trigger an invokation of this callback, and the callback will be deleted afterwards. If your request results in more than one response, consider using
    ///   permanent callbacks (see below) to deal with them.
    ///   Depending on the selected InboundDispatching for this RetMsgData (or the default if none is chosen), the callback can either be called instantly, or placed in a dispatch queue.
    ///   Check the 'Threading Behaviour' section below for more info on this.
    ///   Another thing to keep in mind is that crier has no way to discern if a received message of RetMsgData is the actual response for the sent ReqMsgData (this would be highly 
    ///   complex as it's very application specific). The potential side effects of this is that if you send two messages, one right after the other, and they both expect the same 
    ///   message as a reponse, AND the service your talking with doesn't guarantee that it will treat them in order (or if you use UDP beneath your transport class which doesn't 
    ///   guarantee order of delivery), then it's possible that the RetMsgData reponse for the second message arrives before the reply of the first, in which case, crier will give it 
    ///   to the callback registered with the first message. Consider using permanent callbacks (and application specific code in those callbacks) to deal with these situations, if
    ///   at all possible in your conditions.
    template <typename ReqMsgData, typename RetMsgData>
    void sendMessageWithRetCallback(const ReqMsgData& data, const std::function<void(const RetMsgData&)>& onSuccess);
    
    /// Sends a message of type MsgData, and registers a callback expecting a response of type RetMsgData, as well a callback to run if a response doesn't arrive in the specified time-frame
    /// Both the MsgData and RetMsgData classes must be present in your generated protocol buffer .cc and .h, and they must be contained in your root message (ProtoRootMsg template)
    /// as optional members (or through protobuf extension, check readme for an example of how your root message should look like).
    /// Check the above method documentation for full information on the onSuccess parameter
    /// - milliseconds_to_timeout, time, in milliseconds, to wait for a response of type RetMsgData. If no response arrives in time, the response callback is deleted, and onTimeout is called
    /// - onTimeout, will be called if time to get a response expires. Depending on the selected InboundDispatching for this RetMsgData (or the default if none is chosen), 
    ///   the timeout callback can either be called instantly, or placed in a dispatch queue. Check the 'Threading Behaviour' section below for more info on this.
    ///   Keep in mind: Timeouts depend on a separate thread which is launched as soon as the message is sent. So don't panic if a unknown thread shows up in your instrumentation
    template <typename ReqMsgData, typename RetMsgData>
    void sendMessageWithRetCallbackAndTimeout(const ReqMsgData& data, const std::function<void(const RetMsgData&)>& onSuccess,
                                              unsigned int milliseconds_to_timeout, const std::function<void()>& onTimeout);
    
// -- Permanent Messsage Callbacks
    template <typename Msg>
    void clearCallbacksForMsg();
    
    template <typename RetMsgData, CallbackPriority priority = CallbackPriority::NORMAL>
    void registerPermanentCallback(const std::string &key, const std::function<void(const RetMsgData&)>& onSuccess);
    
    template <typename RetMsgData, CallbackPriority priority = CallbackPriority::NORMAL>
    void clearPermanentCallback(const std::string &key);

// -- Transport Event Callbacks
    template <CallbackPriority priority = CallbackPriority::NORMAL>
    void registerForTransportClosedCallback(const std::string &key, const std::function<void(const std::string&)>& onDisconnect);
    
    template <CallbackPriority priority = CallbackPriority::NORMAL>
    void clearTransportClosedCallback(const std::string &key);
    
    template <CallbackPriority priority = CallbackPriority::NORMAL>
    void registerForTransportOpenedCallback(const std::string &key, const std::function<void()>& onConnect);
    
    template <CallbackPriority priority = CallbackPriority::NORMAL>
    void clearTransportOpenedCallback(const std::string &key);

// -- Unhandled Behaviour
  public:
    template <typename Msg>
    void setUnhandledBehaviourForMsg(UnhandledMessageBehaviour behaviour, bool clearQueue = false);
    
    template <typename Msg>
    void setUnhandledBehaviourForMsgToDefault(bool clearQueue = false);
    
    
// -- Threading Behaviour
    template <typename Msg>
    void setInboundDispatchingForMsg(InboundDispatching behaviour);
    
    template <typename Msg>
    void setInboundDispatchingForMsgToDefault();
    
    void setInboundDispatchingForSocketOpen(InboundDispatching behaviour);
    
    void setInboundDispatchingForSocketClosed(InboundDispatching behaviour);
    
    void setInboundDispatchingForSocketOpenToDefault();
    
    void setInboundDispatchingForSocketClosedToDefault();
    
    InboundDispatching getInboundDispatchingForMsg(const std::string& type);
    
    void dispatchQueuedCallbacks();
    
// -- Transport Error Callback Supression
    template <typename Msg>
    void supressTransportClosedAfterMsgOfType();
    
    void clearSupressionTransportClosed();
    
// -- Serialization Processing
    void SetCustomSerializationFun(const std::function<std::string(const ProtoRootMsg&)>& fun);
    void ClearCustomSerializationFun();

    void SetCustomDeserializationFun(const std::function<ProtoRootMsg(const std::string&)>& fun);
    void ClearCustomDeserializationFun();
    
  private:
#include <crier/private/Crier_priv.hpp>
  };
  
}

#include <crier/private/Crier_impl.hpp>

#endif