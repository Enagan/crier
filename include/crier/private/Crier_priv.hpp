#ifndef CRIER_PRIV_HPP
#define CRIER_PRIV_HPP

// --- Inbound
void receiveMessage(const ProtoContainerMsg& r, google::protobuf::Message* received_msg);
google::protobuf::Message* openReq(const ProtoContainerMsg& r);

void unhandledMessage(const ProtoContainerMsg& r, const std::string& type, UnhandledMessageBehaviour behaviour);
void dealWithUnhandledMessage(const ProtoContainerMsg& r, const std::string& type);

void triggerCallbacksForMsg(const ProtoContainerMsg& r, google::protobuf::Message* received_msg);
void callOnMainThread(const std::function<void()>& callback);
void treatQueuedMessagesForType(const std::string& ret_type);

// --- Outbound
template <typename MsgData>
void packageIntoReq(ProtoContainerMsg& req, const MsgData& data);

// --- Schedule Timeouts
void scheduleTimeout(const std::string& ret_type, unsigned int miliseconds_to_timeout, const std::function<void()>& onTimeout);
void invalidateFirstTimeout(const std::string& ret_type);
void invalidateAllTimeoutsForMsg(const std::string& ret_type);
void invalidateAllTimeouts();

// --- Transport Callbacks
void OnTransportConnect();
void OnTransportData(const std::string& data);
void OnTransportDisconnect(const std::string& err);

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

std::map<std::string, std::deque<ProtoContainerMsg>> _unhandledMessageQueue;
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

std::function<std::string(const ProtoContainerMsg&)> _custom_serialization_fun;
std::function<ProtoContainerMsg(const std::string&)> _custom_deserialization_fun;

#endif