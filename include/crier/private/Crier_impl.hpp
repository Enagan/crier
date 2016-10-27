#ifndef CRIER_IMPL_HPP
#define CRIER_IMPL_HPP

#include <iostream>

namespace crier {

  template <typename Transport, typename ProtoRootMsg>
  Crier<Transport, ProtoRootMsg>::Crier(UnhandledMessageBehaviour default_unhandled_behaviour,
        InboundDispatching default_inbound_dispatch) :
  _transport(new Transport()), _timeoutIds(0), _default_unhandled_behaviour(default_unhandled_behaviour), _default_inbound_dispatch(default_inbound_dispatch),
  _inboundDispatchTransportOpenSetting(default_inbound_dispatch), _inboundDispatchTransportErrorSetting(default_inbound_dispatch),
  _supressNextTransportClosed(false), _custom_serialization_fun(nullptr), _custom_deserialization_fun(nullptr) {
    _transport->setOnConnectCallback([this](){ OnTransportConnect(); });
    _transport->setOnDataCallback([this](const std::string& data){ OnTransportData(data); });
    _transport->setOnDisconnectCallback([this](const std::string& reason){ OnTransportDisconnect(reason); });
  }

  template <typename Transport, typename ProtoRootMsg>
  Crier<Transport, ProtoRootMsg>::Crier(Transport transport, UnhandledMessageBehaviour default_unhandled_behaviour,
          InboundDispatching default_inbound_dispatch) :
  _transport(new Transport(std::move(transport))), _timeoutIds(0), _default_unhandled_behaviour(default_unhandled_behaviour), _default_inbound_dispatch(default_inbound_dispatch),
  _inboundDispatchTransportOpenSetting(default_inbound_dispatch), _inboundDispatchTransportErrorSetting(default_inbound_dispatch),
  _supressNextTransportClosed(false), _custom_serialization_fun(nullptr), _custom_deserialization_fun(nullptr) {
    _transport->setOnConnectCallback([this](){ OnTransportConnect(); });
    _transport->setOnDataCallback([this](const std::string& data){ OnTransportData(data); });
    _transport->setOnDisconnectCallback([this](const std::string& reason){ OnTransportDisconnect(reason); });
  }

  template <typename Transport, typename ProtoRootMsg>
  Crier<Transport, ProtoRootMsg>::~Crier() {
    invalidateAllTimeouts();
    for(auto& thread : _launchedThreads) {
      if(thread.joinable()) thread.join();
    }
  }
  
  template <typename Transport, typename ProtoRootMsg>
  const Transport& Crier<Transport, ProtoRootMsg>::ctransport() const {
    return *_transport;
  }

  template <typename Transport, typename ProtoRootMsg>
  Transport& Crier<Transport, ProtoRootMsg>::transport() {
    return *_transport;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::connectTransport(const std::string& ip, unsigned int port) {
    _supressNextTransportClosed = false;
    _transport->connect(ip, port);
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::disconnectTransport() {
    _transport->disconnect();
  }

  template <typename Transport, typename ProtoRootMsg>
  bool Crier<Transport, ProtoRootMsg>::transportConnected() const {
    return _transport->isConnected();
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename MsgData>
  void Crier<Transport, ProtoRootMsg>::sendMessage(const MsgData& data) {
    ProtoRootMsg req;
    packageIntoReq(req, data);

    if(_custom_serialization_fun) {
      return _transport->sendData(_custom_serialization_fun(req));
    } else {
      return _transport->sendData(req.SerializeAsString());
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename ReqMsgData, typename RetMsgData>
  void Crier<Transport, ProtoRootMsg>::sendMessageWithRetCallback(const ReqMsgData& data, const std::function<void(const RetMsgData&)>& onSuccess) {
    {
      std::lock_guard<std::mutex> guard(_callbackMapMutex);
      _callbackMap[RetMsgData().GetDescriptor()->full_name()].emplace_back([onSuccess](google::protobuf::Message* received_msg){
        onSuccess(*(dynamic_cast<RetMsgData*>(received_msg)));});
    }
    return sendMessage<ReqMsgData>(data);
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename ReqMsgData, typename RetMsgData>
  void Crier<Transport, ProtoRootMsg>::sendMessageWithRetCallbackAndTimeout(const ReqMsgData& data, const std::function<void(const RetMsgData&)>& onSuccess, unsigned int milliseconds_to_timeout, const std::function<void()>& onTimeout) {
    scheduleTimeout(RetMsgData().GetDescriptor()->full_name(), milliseconds_to_timeout, onTimeout);
    return sendMessageWithRetCallback<ReqMsgData, RetMsgData>(data, onSuccess);
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::scheduleTimeout(const std::string& ret_type, unsigned int milliseconds_to_timeout, const std::function<void()>& onTimeout) {
    std::lock_guard<std::mutex> guard(_timeoutCallbackMapMutex);
    _timeoutCallbackMap[ret_type].push_back(TimeoutData{_timeoutIds++, true, onTimeout});
    auto async_timeout_pointer = --_timeoutCallbackMap[ret_type].end();

    std::thread timeout([this, ret_type, async_timeout_pointer, milliseconds_to_timeout]() {
      std::this_thread::sleep_for(std::chrono::milliseconds{milliseconds_to_timeout});

      bool valid;
      std::function<void()> callback;
      {
        std::lock_guard<std::mutex> guard(_timeoutCallbackMapMutex);
        valid = async_timeout_pointer->valid;
        if(valid)
        {
          {
            std::lock_guard<std::mutex> guard(this->_callbackMapMutex);
            // TODO: This is still wrong: if two requests are made for the same type and the second has a smaller timeout and it expires,
            // then the first callback will be removed and the second callback can be called upon the arrival of the first response
            if(this->_callbackMap[ret_type].size() > 0) {
              this->_callbackMap[ret_type].pop_front();
            }
          }
          callback = async_timeout_pointer->callback;
        }
        async_timeout_pointer->valid = false;
        auto timeoutId = async_timeout_pointer->id;
        this->_timeoutCallbackMap[ret_type].remove_if([timeoutId](const TimeoutData& elem){ return elem.id == timeoutId; });
      }

      if(valid) {
        InboundDispatching behaviour = _default_inbound_dispatch;
        {
          std::lock_guard<std::mutex> guard(_inboundDispatchSettingsMutex);
          if(_inboundDispatchSettings.find(ret_type) != _inboundDispatchSettings.end()){
            behaviour = _inboundDispatchSettings[ret_type];
          }
        }
        if(behaviour == InboundDispatching::DispatchQueue)
          callOnMainThread(callback);
        else
          callback();
      }
    });
    _launchedThreads.push_back(std::move(timeout));
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::invalidateFirstTimeout(const std::string& ret_type) {
    std::lock_guard<std::mutex> guard(_timeoutCallbackMapMutex);

    for(auto& timeout_pair : _timeoutCallbackMap[ret_type])
    {
      if(timeout_pair.valid == true)
      {
        timeout_pair.valid = false;
        break;
      }
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::invalidateAllTimeoutsForMsg(const std::string& ret_type) {
    std::lock_guard<std::mutex> guard(_timeoutCallbackMapMutex);

    for(auto& timeout_pair : _timeoutCallbackMap[ret_type])
    {
      timeout_pair.valid = false;
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::invalidateAllTimeouts() {
    std::lock_guard<std::mutex> guard(_timeoutCallbackMapMutex);

    for(auto& timeout_key_val : _timeoutCallbackMap)
    {
      for(auto& timeout_pair : timeout_key_val.second)
      {
        timeout_pair.valid = false;
      }
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename Msg>
  void Crier<Transport, ProtoRootMsg>::setUnhandledBehaviourForMsg(UnhandledMessageBehaviour behaviour) {
    std::string ret_type = Msg().GetDescriptor()->full_name();
    std::lock_guard<std::mutex> guardBehaviour(_unhandledBehaviourSettingsMutex);
    _unhandledBehaviourSettings[ret_type] = behaviour;

    if (behaviour == UnhandledMessageBehaviour::Ignore) {
      std::lock_guard<std::mutex> guardQueue(_unhandledMessageQueueMutex);
      _unhandledMessageQueue[ret_type].clear();
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename Msg>
  void Crier<Transport, ProtoRootMsg>::setUnhandledBehaviourForMsgToDefault() {
    setUnhandledBehaviourForMsg<Msg>(_default_unhandled_behaviour);
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename Msg>
  void Crier<Transport, ProtoRootMsg>::setInboundDispatchingForMsg(InboundDispatching behaviour) {
    std::string ret_type = Msg().GetDescriptor()->full_name();
    std::lock_guard<std::mutex> guardBehaviour(_inboundDispatchSettingsMutex);
    _inboundDispatchSettings[ret_type] = behaviour;
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename Msg>
  void Crier<Transport, ProtoRootMsg>::setInboundDispatchingForMsgToDefault() {
    setInboundDispatchingForMsg<Msg>(_default_inbound_dispatch);
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::setInboundDispatchingForTransportOpen(InboundDispatching behaviour) {
    _inboundDispatchTransportOpenSetting = behaviour;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::setInboundDispatchingForTransportClosed(InboundDispatching behaviour) {
    _inboundDispatchTransportErrorSetting = behaviour;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::setInboundDispatchingForTransportOpenToDefault() {
    _inboundDispatchTransportOpenSetting = _default_inbound_dispatch;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::setInboundDispatchingForTransportClosedToDefault() {
    _inboundDispatchTransportErrorSetting = _default_inbound_dispatch;
  }

  template <typename Transport, typename ProtoRootMsg>
  InboundDispatching Crier<Transport, ProtoRootMsg>::getInboundDispatchingForMsg(const std::string& type) {
    std::lock_guard<std::mutex> guard(_inboundDispatchSettingsMutex);
    if(_inboundDispatchSettings.find(type) != _inboundDispatchSettings.end()){
      return _inboundDispatchSettings[type];
    }
    return _default_inbound_dispatch;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::dispatchQueuedCallbacks() {
    std::deque<std::function<void()>> mainThreadCallbacksAux;
    {
      std::lock_guard<std::mutex> guard(_mainThreadCallbacksMapMutex);
      mainThreadCallbacksAux = std::move(_mainThreadCallbacksMap);
      _mainThreadCallbacksMap.clear();
    }
    for(const auto& callback : mainThreadCallbacksAux)
    {
      callback();
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename Msg>
  void Crier<Transport, ProtoRootMsg>::clearCallbacksForMsg() {
    std::string ret_type = Msg().GetDescriptor()->full_name();
    std::lock_guard<std::mutex> guard(_callbackMapMutex);
    _callbackMap[ret_type].clear();
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename RetMsgData, CallbackPriority priority>
  void Crier<Transport, ProtoRootMsg>::registerPermanentCallback(const std::string &key, const std::function<void(const RetMsgData&)>& onSuccess) {
    std::string ret_type = RetMsgData().GetDescriptor()->full_name();
    {
      std::lock_guard<std::mutex> guard(_permanentObserverMapMutex);
      _permanentObserverMap[ret_type][{key, priority}] = [onSuccess](google::protobuf::Message* received_msg){
        onSuccess(*(dynamic_cast<RetMsgData*>(received_msg)));};
    }

    treatQueuedMessagesForType(ret_type);
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename RetMsgData, CallbackPriority priority>
  void Crier<Transport, ProtoRootMsg>::clearPermanentCallback(const std::string &key){
    std::string ret_type = RetMsgData().GetDescriptor()->full_name();
    std::lock_guard<std::mutex> guard(_permanentObserverMapMutex);
    _permanentObserverMap[ret_type].erase({key, priority});
  }

  template <typename Transport, typename ProtoRootMsg>
  template <CallbackPriority priority>
  void Crier<Transport, ProtoRootMsg>::registerForTransportClosedCallback(const std::string &key, const std::function<void(const std::string&)>& onDisconnect) {
    std::lock_guard<std::mutex> guard(_transportClosedObserverMapMutex);
    _transportClosedObserverMap[{key, priority}] = onDisconnect;
  }

  template <typename Transport, typename ProtoRootMsg>
  template <CallbackPriority priority>
  void Crier<Transport, ProtoRootMsg>::clearTransportClosedCallback(const std::string &key) {
    std::lock_guard<std::mutex> guard(_transportClosedObserverMapMutex);
    _transportClosedObserverMap.erase({key, priority});
  }

  template <typename Transport, typename ProtoRootMsg>
  template <CallbackPriority priority>
  void Crier<Transport, ProtoRootMsg>::registerForTransportOpenedCallback(const std::string &key, const std::function<void()>& onConnect) {
    std::lock_guard<std::mutex> guard(_transportOpenedObserverMapMutex);
    _transportOpenedObserverMap[{key, priority}] = onConnect;
  }

  template <typename Transport, typename ProtoRootMsg>
  template <CallbackPriority priority>
  void Crier<Transport, ProtoRootMsg>::clearTransportOpenedCallback(const std::string &key) {
    std::lock_guard<std::mutex> guard(_transportOpenedObserverMapMutex);
    _transportOpenedObserverMap.erase({key, priority});
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename Msg>
  void Crier<Transport, ProtoRootMsg>::supressTransportClosedAfterMsgOfType() {
    std::string ret_type = Msg().GetDescriptor()->full_name();
    _transportClosedSupressors[ret_type] = true;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::clearSupressionTransportClosed() {
    _transportClosedSupressors.clear();
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::unhandledMessage(const ProtoRootMsg& r, const std::string& type, UnhandledMessageBehaviour behaviour) {
    if(behaviour == UnhandledMessageBehaviour::Ignore) {
      // Do nothing
    }
    else if(behaviour == UnhandledMessageBehaviour::Enqueue){
      std::lock_guard<std::mutex> guard(_unhandledMessageQueueMutex);
      _unhandledMessageQueue[type].push_back(r); // TODO NEEDS DEEP COPY
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::callOnMainThread(const std::function<void()>& callback) {
    std::lock_guard<std::mutex> guard(_mainThreadCallbacksMapMutex);
    _mainThreadCallbacksMap.push_back(callback);
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::receiveMessage(const ProtoRootMsg& r, google::protobuf::Message* received_msg) {
    std::string type = received_msg->GetDescriptor()->full_name();
    invalidateFirstTimeout(type);

    if(_transportClosedSupressors[type])
      _supressNextTransportClosed = true;

    // Get the threading behaviour for this message, to define where it should be called on (main thread, or helper thread)
    InboundDispatching behaviour = getInboundDispatchingForMsg(type);

    if(behaviour == InboundDispatching::Immediate) {
      triggerCallbacksForMsg(r, received_msg);
    }
    else if(behaviour == InboundDispatching::DispatchQueue) {
      callOnMainThread([this, r](){
        auto req_data = openReq(r);
        if(req_data != nullptr)
          triggerCallbacksForMsg(r, req_data);
      });
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::triggerCallbacksForMsg(const ProtoRootMsg& r, google::protobuf::Message* received_msg) {
    std::string type = received_msg->GetDescriptor()->full_name();
    bool no_callbacks = true;
    /// Call all Permanent callbacks
    std::vector<std::function<void(google::protobuf::Message*)>> permanentObserverList;
    {
      std::lock_guard<std::mutex> guard(_permanentObserverMapMutex);
      permanentObserverList = mapToVectorCopy(_permanentObserverMap[type]);
    }
    for (const auto& permObserver : permanentObserverList) {
      permObserver(received_msg);

      no_callbacks = false;
    }

    /// Call first Temporary callback
    _callbackMapMutex.lock();
    // This has some unexpected behaviour: if there are two requests made in succession, the first without a callback and the second with a callback,
    // then the calback will be used for the first request and not for the second
    if(_callbackMap[type].size() > 0) {
      auto callback = _callbackMap[type].front();
      _callbackMap[type].pop_front();
      _callbackMapMutex.unlock();     // UNLOCK _callbackMapMutex
      callback(received_msg);
      no_callbacks = false;
    } else {
      _callbackMapMutex.unlock();
    }

    if(no_callbacks) {
      dealWithUnhandledMessage(r, type);
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::dealWithUnhandledMessage(const ProtoRootMsg& r, const std::string& type) {
    UnhandledMessageBehaviour unhandled_behaviour = _default_unhandled_behaviour;
    {
      std::lock_guard<std::mutex> guard(_unhandledBehaviourSettingsMutex);
      if(_unhandledBehaviourSettings.find(type) != _unhandledBehaviourSettings.end()) {
        unhandled_behaviour = _unhandledBehaviourSettings[type];
      }
    }
    unhandledMessage(r, type, unhandled_behaviour);
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::OnTransportData(const std::string& data) {
    ProtoRootMsg container_msg;

    if(_custom_deserialization_fun) {
      container_msg = _custom_deserialization_fun(data);
    } else {
      container_msg.ParseFromString(data);
    }

    google::protobuf::Message* msg_data = openReq(container_msg);
    if(msg_data != nullptr)
      receiveMessage(container_msg, msg_data);
  }

  template <typename Transport, typename ProtoRootMsg>
  google::protobuf::Message* Crier<Transport, ProtoRootMsg>::openReq(const ProtoRootMsg& r) {

    const google::protobuf::Reflection *refl = r.GetReflection();

    std::vector< const google::protobuf::FieldDescriptor *> pOut;

    refl->ListFields( r, &pOut );

    for( auto const &field : pOut )
    {
      if( field == nullptr ) { continue; };

      google::protobuf::Message* msgPointer = refl->MutableMessage((google::protobuf::Message*)&r, field);
      return msgPointer;
    }

    logEmptyMessageError();
    return nullptr;
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename MsgData>
  void Crier<Transport, ProtoRootMsg>::packageIntoReq(ProtoRootMsg& req, const MsgData& data) {

    const google::protobuf::Descriptor* data_desc	= data.GetDescriptor();
    const google::protobuf::Descriptor* req_desc	= req.GetDescriptor();
    const google::protobuf::Reflection* req_refl	= req.GetReflection();

    int fieldCount = req_desc->field_count();
    for( int i = 0; i<fieldCount; i++ )
    {
      const google::protobuf::FieldDescriptor *field = req_desc->field(i);

      if(( field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ) && ( field->message_type()->full_name() == data_desc->full_name() ))
      {
        MsgData* msgPointer = (MsgData*)req_refl->MutableMessage((google::protobuf::Message*)&req, field);
        msgPointer->CopyFrom(data);
        return;
      }
    }

    // Search through Extensions at File Level:
    auto file_data_desc = data_desc->file();

    int extCount = file_data_desc->extension_count();
    for( int i = 0; i < extCount; i++ )
    {
      const google::protobuf::FieldDescriptor *field = file_data_desc->extension(i);

      if(( field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE ) &&
         ( field->containing_type()->full_name() == req_desc->full_name() ) &&
         ( field->message_type()->full_name() == data_desc->full_name() ))
      {
        MsgData* msgPointer = (MsgData*)req_refl->MutableMessage((google::protobuf::Message*)&req, field);
        msgPointer->CopyFrom(data);
        return;
      }
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::OnTransportConnect(){
    std::vector<std::function<void()>> socketOpenedObserverList;
    {
      std::lock_guard<std::mutex> guard(_transportOpenedObserverMapMutex);
      socketOpenedObserverList = mapToVectorCopy(_transportOpenedObserverMap);
    }
    if(_inboundDispatchTransportOpenSetting == InboundDispatching::DispatchQueue) {
      for (const auto& callback : socketOpenedObserverList) {
        callOnMainThread(callback);
      }
    }
    else {
      for (const auto& callback : socketOpenedObserverList) {
        callback();
      }
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::OnTransportDisconnect(const std::string& err) {
    if(_supressNextTransportClosed) {
      _supressNextTransportClosed = false;
      return;
    }

    std::vector<std::function<void(const std::string&)>> socketClosedObserverList;
    {
      std::lock_guard<std::mutex> guard(_transportClosedObserverMapMutex);
      socketClosedObserverList = mapToVectorCopy(_transportClosedObserverMap);
    }

    if(_inboundDispatchTransportErrorSetting == InboundDispatching::DispatchQueue) {
      for (const auto& callback : socketClosedObserverList) {
        callOnMainThread([err, callback](){callback(err);});
      }
    } else {
      for (const auto& callback : socketClosedObserverList) {
        callback(err);
      }
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::treatQueuedMessagesForType(const std::string& ret_type) {
    std::deque<ProtoRootMsg> unhandledMessageAux;
    {
      std::lock_guard<std::mutex> guard(_unhandledMessageQueueMutex);
      unhandledMessageAux = std::move(_unhandledMessageQueue[ret_type]);
      _unhandledMessageQueue[ret_type].clear();
    }

    for(const auto& queued_msg : unhandledMessageAux) {
      auto req_data = openReq(queued_msg);
      if(req_data != nullptr)
        receiveMessage(queued_msg, req_data);
    }
  }

  template <typename Transport, typename ProtoRootMsg>
  inline void Crier<Transport, ProtoRootMsg>::logEmptyMessageError() {
    std::cout << "[CRIER] ERROR: Couldn't Parse message it appears to have arrived empty" << std::endl;
  }

  template <typename Transport, typename ProtoRootMsg>
  template <typename CallbackType>
  std::vector<CallbackType> Crier<Transport, ProtoRootMsg>::mapToVectorCopy(const CallbackMap<CallbackType>& source) {
    std::vector<CallbackType> retVal;
    retVal.reserve(source.size());
    for (const auto& MapElem : source) {
      retVal.push_back(MapElem.second);
    }
    return retVal;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::SetCustomSerializationFun(const std::function<std::string(const ProtoRootMsg&)>& fun) {
    _custom_serialization_fun = fun;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::ClearCustomSerializationFun() {
    _custom_serialization_fun = nullptr;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::SetCustomDeserializationFun(const std::function<ProtoRootMsg(const std::string&)>& fun) {
    _custom_deserialization_fun = fun;
  }

  template <typename Transport, typename ProtoRootMsg>
  void Crier<Transport, ProtoRootMsg>::ClearCustomDeserializationFun() {
    _custom_deserialization_fun = nullptr;
  }

}

#endif
