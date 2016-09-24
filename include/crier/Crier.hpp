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
    //  - default_unhandled_behaviour, specifies what crier should do when receiving a message that has 0 registered callbacks to listen to it. By default, the option is ignore
    //    which will simply discard the message, the other option in enqueue, which will save the message to be dispatched when the first permanent callback for it is registered.
    //  - default_inbound_dispatch, specifies how crier should invoke the callbacks when messages arrive, the default is 'Immediate', which will call the lambdas as soon as messages arrive
    //    through the transport. This means, for example, if your transport inbound runs on a separate thread, your callbacks will run on that thread (crier is thread-safe, but the
    //    side-effects of your callbacks might not be, so take care). The other option is 'DispatchQueue', which will save the callbacks in a queue which is only dispatched when (and where)
    //    you call the 'dispatchQueuedCallbacks' method.
    //  When using this constructor, keep in mind that crier will create it's own instance of the Transport class it was templated with, which means that class must have a default constructor.
    //  If having a default constructor is impossible or impractical for whatever reason, check the other constructor below.
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

    /// Gets a reference to the crier managed transport instance. Typically you shouldn't mess with it during regular usage, but it's available for edge cases
    Transport& transport();

// -- Message Sends
// Methods to send protobuf messages through the transport

    /// Sends a message of type MsgData.
    /// The MsgData class must be present in your generated protocol buffer .cc and .h, and it must be contained in your root message (ProtoRootMsg template)
    /// as an optional member (or through a protobuf extension, check readme for an example of how your root message should look like).
    /// If these requirements are not met, crier won't be able to bundle the message and won't do anything.
    template <typename MsgData>
    void sendMessage(const MsgData& data);

    /// Sends a message of type MsgData, and registers a callback, expecting a response of type RetMsgData.
    /// Both the MsgData and RetMsgData classes must be present in your generated protocol buffer .cc and .h, and they must be contained in your root message (ProtoRootMsg template)
    /// as optional members (or through protobuf extension, check readme for an example of how your root message should look like).
    //  - data, the object of type ReqMsgData that you want to send
    //  - onSuccess, callback receiving a RetMsgData as a response. Keep in mind that this is seen as a "consumable" callback for crier. This means the first message to arrive of
    //    RetMsgData type will trigger an invokation of this callback, and the callback will be deleted afterwards. If your request results in more than one response, consider using
    //    permanent callbacks (see below) to deal with them.
    //    Depending on the selected InboundDispatching for this RetMsgData (or the default if none is chosen), the callback can either be called instantly, or placed in a dispatch queue.
    //    Check the 'Threading Behaviour' section below for more info on this.
    //    Another thing to keep in mind is that crier has no way to discern if a received message of RetMsgData is the actual response for the sent ReqMsgData (this would be highly
    //    complex as it's very application specific). The potential side effects of this is that if you send two messages, one right after the other, and they both expect the same
    //    message as a reponse, AND the service your talking with doesn't guarantee that it will treat them in order (or if you use UDP beneath your transport class which doesn't
    //    guarantee order of delivery), then it's possible that the RetMsgData reponse for the second message arrives before the reply of the first, in which case, crier will give it
    //    to the callback registered with the first message. Consider using permanent callbacks (and application specific code in those callbacks) to deal with these situations, if
    //    at all possible in your conditions.
    template <typename ReqMsgData, typename RetMsgData>
    void sendMessageWithRetCallback(const ReqMsgData& data, const std::function<void(const RetMsgData&)>& onSuccess);

    /// Sends a message of type MsgData, and registers a callback expecting a response of type RetMsgData, as well a callback to run if a response doesn't arrive in the specified time-frame
    /// Both the MsgData and RetMsgData classes must be present in your generated protocol buffer .cc and .h, and they must be contained in your root message (ProtoRootMsg template)
    /// as optional members (or through protobuf extension, check readme for an example of how your root message should look like).
    /// Check the above method documentation for full information on the onSuccess parameter.
    //  - milliseconds_to_timeout, time, in milliseconds, to wait for a response of type RetMsgData. If no response arrives in time, the response callback is deleted, and onTimeout is called
    //  - onTimeout, will be called if time to get a response expires. Depending on the selected InboundDispatching for this RetMsgData (or the default if none is chosen),
    //    the timeout callback can either be called instantly, or placed in a dispatch queue. Check the 'Threading Behaviour' section below for more info on this.
    //    Keep in mind: Timeouts depend on a separate thread which is launched as soon as the message is sent. So don't panic if a unknown thread shows up in your instrumentation
    template <typename ReqMsgData, typename RetMsgData>
    void sendMessageWithRetCallbackAndTimeout(const ReqMsgData& data, const std::function<void(const RetMsgData&)>& onSuccess,
                                                unsigned int milliseconds_to_timeout, const std::function<void()>& onTimeout);

// -- Permanent Messsage Callbacks
// Methods to deal with permanent callbacks for protobuf messages.

    /// Registers a permanent callback to run every time a message of type RetMsgData arrives through the transport.
    //  - priority is an optional template parameter that controls when the callback will be invoked in relation to other callbacks registered for the same RetMsgData.
    //    There are three available priorities, FIRST, ASAP and NORMAL. All callbacks for FIRST will run before any ASAP callback, and the same applies between ASAP and NORMAL.
    //    You can register two callbacks for the same message in different priorities, and they will run as expected in relation to one another.
    //  - key should be an unique identifier for the callback being registered. This key is for later use in clearing the callback away. If two callbacks are registered for the same
    //    RetMsgData using the same key, the second will overwrite the first. (The same key can be used for two different RetMsgData).
    //  - onSuccess is the callback to be invoked upon arrival of the RetMsgData.
    template <typename RetMsgData, CallbackPriority priority = CallbackPriority::NORMAL>
    void registerPermanentCallback(const std::string& key, const std::function<void(const RetMsgData&)>& onSuccess);

    /// Clears a permanent callback that was set to run every time a message of type RetMsgData arrives through the transport.
    /// To correctly clear a callback, make sure that the template argument for the priority is the same, as well as the key used.
    template <typename RetMsgData, CallbackPriority priority = CallbackPriority::NORMAL>
    void clearPermanentCallback(const std::string& key);

    /// Clears all permanent callbacks for the specified message, across all priorities.
    template <typename Msg>
    void clearCallbacksForMsg();

// -- Transport Event Callbacks
// Methods to deal with permanent callbacks for transport events.

    /// Registers a permanent callback to run every time the Transport is successfully connected (will trigger when your Transport Concept implementation calls it's
    /// _on_connect_cb, so it's up to the implementer to define what a Transport connected means).
    //  - priority is an optional template parameter that controls when the callback will be invoked in relation to other callbacks registered for Transport open.
    //    There are three available priorities, FIRST, ASAP and NORMAL. All callbacks for FIRST will run before any ASAP callback, and the same applies between ASAP and NORMAL.
    //    You can register two different callbacks in different priorities, and they will run as expected in relation to one another.
    //  - key should be an unique identifier for the callback being registered. This key is for later use in clearing the callback away. If two callbacks are registered at the same
    //    priority level using the same key, the second will overwrite the first.
    //  - onConnect is the callback to be invoked when the transport connected event happens.
    template <CallbackPriority priority = CallbackPriority::NORMAL>
    void registerForTransportOpenedCallback(const std::string &key, const std::function<void()>& onConnect);

    /// Registers a permanent callback to run every time the Transport is disconnected (will trigger when your Transport Concept implementation calls it's
    /// _on_disconnect_cb, so it's up to the implementer to define what a Transport disconnect means).
    //  - priority is an optional template parameter that controls when the callback will be invoked in relation to other callbacks registered for transport disconnect.
    //    There are three available priorities, FIRST, ASAP and NORMAL. All callbacks for FIRST will run before any ASAP callback, and the same applies between ASAP and NORMAL.
    //    You can register two different callbacks in different priorities, and they will run as expected in relation to one another.
    //  - key should be an unique identifier for the callback being registered. This key is for later use in clearing the callback away. If two callbacks are registered at the same
    //    priority level using the same key, the second will overwrite the first.
    //  - onDisconnect is the callback to be invoked when the transport disconnect event happens. It must receive a string as input parameter which is passed from the Transport Concept
    //    it's purpose is to describe the error which caused the disconnect in some way.
    template <CallbackPriority priority = CallbackPriority::NORMAL>
    void registerForTransportClosedCallback(const std::string &key, const std::function<void(const std::string&)>& onDisconnect);

    /// Clears a permanent callback that was set to run when transport connected event is triggered.
    // To correctly clear a callback, make sure that the template argument for the priority is the same, as well as the key used.
    template <CallbackPriority priority = CallbackPriority::NORMAL>
    void clearTransportOpenedCallback(const std::string &key);

    /// Clears a permanent callback that was set to run when transport disconnect event is triggered.
    // To correctly clear a callback, make sure that the template argument for the priority is the same, as well as the key used.
    template <CallbackPriority priority = CallbackPriority::NORMAL>
    void clearTransportClosedCallback(const std::string &key);

// -- Unhandled Behaviour
// Methods to modify crier's unhandled behaviour for messages and transport events.

    /// Set a specific unhandled behaviour for messages of type Msg, to override the option set as default.
    /// The unhandled behaviour describes what crier should do when a message of type Msg arrives, but no callback (temporary of permanent) is registered to listen.
    /// Ignore will simply discard the message, Enqueue will save the message in a queue to be dispatched when the first permanent callback for it is registered.
    /// Whenever the behaviour is set to ignore, the current queue of messages of type Msg will be cleared.
    template <typename Msg>
    void setUnhandledBehaviourForMsg(UnhandledMessageBehaviour behaviour);

    /// Resets the unhandled behaviour for messages of type Msg to the default set when constructing the crier instance.
    template <typename Msg>
    void setUnhandledBehaviourForMsgToDefault();

// -- Inbound Dispatching Behaviour
// Methods to modify crier's dispatching behaviour for messages and transport events.

    /// Set a specific dispatching behaviour for messages of type Msg, to override the option set as default.
    /// The dispatching behaviour describes how crier should invoke callbacks when a message of type Msg arrives.
    //  Immediate will call the lambdas as soon as messages arrive through the transport. This means, for example, if your transport inbound runs on a separate thread,
    //  your callbacks will run on that thread (crier is thread-safe, but the side-effects of your callbacks might not be, so take care).
    //  DispatchQueue will save the callbacks in a queue which is only dispatched when (and where) you call the 'dispatchQueuedCallbacks' method.
    template <typename Msg>
    void setInboundDispatchingForMsg(InboundDispatching behaviour);

    /// Resets the dispatching behaviour for messages of type Msg to the default set when constructing the crier instance.
    template <typename Msg>
    void setInboundDispatchingForMsgToDefault();

    /// Set a specific dispatching behaviour for the transport open event, to override the option set as default.
    /// The dispatching behaviour describes how crier should invoke callbacks when the transport open event is triggered.
    //  Immediate will call the lambdas as soon as the event is triggered in the transport (when calling _on_connect_cb).
    //  This means, for example, if your transport inbound runs on a separate thread, your callbacks will run on that thread
    //  (crier is thread-safe, but the side-effects of your callbacks might not be, so take care).
    //  DispatchQueue will save the callbacks in a queue which is only dispatched when (and where) you call the 'dispatchQueuedCallbacks' method.
    void setInboundDispatchingForTransportOpen(InboundDispatching behaviour);

    /// Set a specific dispatching behaviour for the transport closed event, to override the option set as default.
    /// The dispatching behaviour describes how crier should invoke callbacks when the transport closed event is triggered.
    //  Immediate will call the lambdas as soon as the event is triggered in the transport (when calling _on_disconnect_cb).
    //  This means, for example, if your transport inbound runs on a separate thread, your callbacks will run on that thread
    //  (crier is thread-safe, but the side-effects of your callbacks might not be, so take care).
    //  DispatchQueue will save the callbacks in a queue which is only dispatched when (and where) you call the 'dispatchQueuedCallbacks' method.
    void setInboundDispatchingForTransportClosed(InboundDispatching behaviour);

    /// Resets the dispatching behaviour for the transport open event to the default set when constructing the crier instance.
    void setInboundDispatchingForTransportOpenToDefault();

    /// Resets the dispatching behaviour for the transport closed event to the default set when constructing the crier instance.
    void setInboundDispatchingForTransportClosedToDefault();

    /// Invokes any pending callbacks for messages or transport events that are running using the 'DispatchQueue' inbound dispatching option. Callbacks will be invoked in arrival order.
    /// Usefull if you want to control the time and place your external messages are treated.
    /// This method must be called if any messages or events are running with dispatch queue, otherwise their callbacks will never be called.
    //  Keep in mind that messages will be kept in memory waiting for this call. If you receive large messages very frequently, and you have them placed in the dispatch queue, but you don't call
    //  this method very often, you might see an impact in memory used. Similarly, if you have messages entering the queue but you never dispatch, your application will eventually eat all the memory
    //  it can and your OS of choice will most likely kill it. You´ve been warned.
    void dispatchQueuedCallbacks();

// --- Advanced API
// Methods you should be careful when using, as they might have catastrophic outcomes when mis-used

// -- Transport Error Callback Supression
// Suppressing transport closed events in specific situations

    /// There are certain situations when it's desireable to define formal disconnect messages in the protocol (that probably describe the disconnect cause).
    /// These messages are most likely immediately followed by a disconnect in the socket.
    /// This however might be bothersome if you have registered callbacks for both the disconnect message and the transport closed event, since you are now
    /// treating the same disconnect in different places and most likely different contexts.
    /// This option allows you to select a message type, that upon arrival, will suppress the next transport closed event trigger, allowing you to treat formal
    /// disconnects as well as unexpected ones in a cleaner way.
    //  Keep in mind that crier doesn't keep a timer on this suppression. If the transport closed event takes 10 minutes, it will be suppressed.
    //  If this is used with a message that doesn't always result in a transport closed, behaviour can be potential awkward and not what you expected.
    //  (As in, you might miss an event you should not have).
    template <typename Msg>
    void supressTransportClosedAfterMsgOfType();

    /// Turns off the transport closed supressing behaviour
    void clearSupressionTransportClosed();

// -- Serialization Processing
// When you require a more refined Serialization rather than just calling protobuf's SerializeToString.

    /// Sets a function to be called whenever crier needs to Serialize a ProtoRootMsg. Receives a ProtoRootMsg and should return a string with the serialized data.
    /// By default crier will call protobuf's SerializeToString. If you require something more sophisticated, then pass the function with your serialization process.
    void SetCustomSerializationFun(const std::function<std::string(const ProtoRootMsg&)>& fun);

    /// Clears away the set custom serialization function, returning to crier's base behaviour of just calling protobuf's SerializeToString.
    void ClearCustomSerializationFun();

    /// Sets a function to be called whenever crier attempts to deserialize a string into a ProtoRootMsg. Receives a string and should return a ProtoRootMsg deserialized from the string.
    /// By default crier will call protobuf's ParseFromString. If you require something more sophisticated, then pass the function with your deserialization process.
    void SetCustomDeserializationFun(const std::function<ProtoRootMsg(const std::string&)>& fun);

    /// Clears away the set custom deserialization function, returning to crier's base behaviour of just calling protobuf's ParseFromString.
    void ClearCustomDeserializationFun();

  private:
#include <crier/private/Crier_priv.hpp>
  };
}

#include <crier/private/Crier_impl.hpp>

#endif
