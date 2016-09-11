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
  
  template <typename Transport, typename ProtoContainerMsg>
  class Crier {
    
    struct TimeoutData {
      unsigned int id;
      bool valid;
      std::function<void()> callback;
    };
    
    using PriorityKeyPair = std::pair< std::string, CallbackPriority >;
    struct PriorityKeyCompare {
      bool operator()(const PriorityKeyPair& a, const PriorityKeyPair& b) const {
        if(a.second == b.second)
          return a.first < b.first;
        return a.second == CallbackPriority::FIRST || (a.second == CallbackPriority::ASAP && b.second == CallbackPriority::NORMAL);
      }
    };
    
    template <typename CallbackType>
    using CallbackMap = typename std::map< PriorityKeyPair, CallbackType, PriorityKeyCompare >;
    using TimeoutList = typename std::list< TimeoutData >;
    
  public:
    Crier(UnhandledMessageBehaviour default_unhandled_behaviour = UnhandledMessageBehaviour::Ignore,
          InboundDispatching default_inbound_dispatch = InboundDispatching::Immediate);
    
    Crier(const Crier& copy) = delete;
    Crier(const Crier&& copy) = delete;
    
    void operator=(const Crier& copy) = delete;
    void operator=(const Crier&& copy) = delete;
    
    ~Crier();
    
// -- API Methods
// -- Transport
    Transport& transport();
    void ConnectTransport(const std::string& ip, unsigned int port);
    void DisconnectTransport();
    bool TransportConnected() const;
    
// -- Message Sends
    template <typename MsgData>
    void sendMessage(const MsgData& data);
    
    template <typename ReqMsgData, typename RetMsgData>
    void sendMessageWithRetCallback(const ReqMsgData& data, const std::function<void(const RetMsgData&)>& onSuccess);
    
    template <typename ReqMsgData, typename RetMsgData>
    void sendMessageWithRetCallbackAndTimeout(const ReqMsgData& data, const std::function<void(const RetMsgData&)>& onSuccess,
                                              unsigned int miliseconds_to_timeout, const std::function<void()>& onTimeout);
    
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
    
    void setInboundDispatchingForSocketOpenToDefault(InboundDispatching behaviour);
    
    void setInboundDispatchingForSocketClosedToDefault(InboundDispatching behaviour);
    
    InboundDispatching getInboundDispatchingForMsg(const std::string& type);
    
    void dispatchCallbacksInMainThread();
    
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
    
// -- Transport Error Callback Supression
    template <typename Msg>
    void supressTransportClosedAfterMsgOfType();
    
    void clearSupressionTransportClosed();
    
// -- Serialization Processing
    void SetCustomSerializationFun(const std::function<std::string(const ProtoContainerMsg&)>& fun);
    void ClearCustomSerializationFun();

    void SetCustomDeserializationFun(const std::function<ProtoContainerMsg(const std::string&)>& fun);
    void ClearCustomDeserializationFun();
    
  private:
#include <crier/private/Crier_priv.hpp>
  };
  
}

#include <crier/private/Crier_impl.hpp>

#endif