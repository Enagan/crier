#ifndef CRIER_PRIV_HPP
#define CRIER_PRIV_HPP

public:
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

  // --- Inbound
  void receiveMessage(const ProtoRootMsg& r, google::protobuf::Message* received_msg);
  google::protobuf::Message* openReq(const ProtoRootMsg& r);

  void unhandledMessage(const ProtoRootMsg& r, const std::string& type, UnhandledMessageBehaviour behaviour);
  void dealWithUnhandledMessage(const ProtoRootMsg& r, const std::string& type);

  void triggerCallbacksForMsg(const ProtoRootMsg& r, google::protobuf::Message* received_msg);
  void callOnMainThread(const std::function<void()>& callback);
  void treatQueuedMessagesForType(const std::string& ret_type);

  // --- Outbound
  template <typename MsgData>
  void packageIntoReq(ProtoRootMsg& req, const MsgData& data);

  // --- Schedule Timeouts
  void scheduleTimeout(const std::string& ret_type, unsigned int miliseconds_to_timeout, const std::function<void()>& onTimeout);
  void invalidateFirstTimeout(const std::string& ret_type);
  void invalidateAllTimeoutsForMsg(const std::string& ret_type);
  void invalidateAllTimeouts();

  // --- Transport Callbacks
  void OnTransportConnect();
  void OnTransportData(const std::string& data);
  void OnTransportDisconnect(const std::string& err);

  // --- Inbound Dispatching
  InboundDispatching getInboundDispatchingForMsg(const std::string& type);

  // - Utils
  inline void logEmptyMessageError();
  template <typename CallbackType>
  std::vector<CallbackType> mapToVectorCopy(const CallbackMap<CallbackType>& source);

  // - Class Variables
  private:
  std::unique_ptr<Transport> _transport;

  std::map<std::string, std::deque<std::function<void(google::protobuf::Message*)>>> _callbackMap;
  std::mutex _callbackMapMutex;

  std::map<std::string, CallbackMap<std::function<void(google::protobuf::Message*)>>> _permanentObserverMap;
  std::mutex _permanentObserverMapMutex;

  std::map<std::string, TimeoutList> _timeoutCallbackMap;
  std::mutex _timeoutCallbackMapMutex;
  unsigned int _timeoutIds;

  std::vector<std::thread> _launchedThreads;

  CallbackMap<std::function<void(const std::string&)>> _transportClosedObserverMap;
  CallbackMap<std::function<void()>> _transportOpenedObserverMap;
  std::mutex _transportClosedObserverMapMutex;
  std::mutex _transportOpenedObserverMapMutex;

  UnhandledMessageBehaviour _default_unhandled_behaviour;
  std::map<std::string, UnhandledMessageBehaviour> _unhandledBehaviourSettings;
  std::mutex _unhandledBehaviourSettingsMutex;

  std::map<std::string, std::deque<ProtoRootMsg>> _unhandledMessageQueue;
  std::mutex _unhandledMessageQueueMutex;

  InboundDispatching _default_inbound_dispatch;
  std::map<std::string, InboundDispatching> _inboundDispatchSettings;
  InboundDispatching _inboundDispatchTransportOpenSetting;
  InboundDispatching _inboundDispatchTransportErrorSetting;
  std::mutex _inboundDispatchSettingsMutex;

  std::deque<std::function<void()>> _mainThreadCallbacksMap;
  std::mutex _mainThreadCallbacksMapMutex;

  std::map<std::string, bool> _transportClosedSupressors;
  bool _supressNextTransportClosed;

  std::function<std::string(const ProtoRootMsg&)> _custom_serialization_fun;
  std::function<ProtoRootMsg(const std::string&)> _custom_deserialization_fun;

#endif
