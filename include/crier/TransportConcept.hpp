#ifndef CRIER_TRANSPORT_HPP
#define CRIER_TRANSPORT_HPP

#include <string>
#include <functional>

namespace crier {

  /// In order to use crier, a Transport class must be written and supplied to the crier instance as a template parameter.
  /// This Transport Concept defines the methods required by crier to accept a transport as it's template argument.
  //  The purpose of this class is to allow the API user to write it as a decorator pattern over whichever kind of transport
  //  he wants to use to communicate (tcp, udp, websocket, outstream, etc...).
  //  Crier will take ownership (or create it's own) instance of this Transport, and use it as it's underlying inbound/outbound.
  //  It is not required to create your transport as a subclass of this, but it might be usefull.
  class TransportConcept {
  public:

    /// Will be called by crier whenever you call it's 'connectTransport(const std::string& ip, unsigned int port)' method.
    virtual void connect(const std::string& host, int ip) = 0;

    /// Will be called by crier whenever you call it's 'disconnectTransport()' method.
    virtual void disconnect() = 0;

    /// Will be called by crier whenever you call it's 'transportConnected()' method. Should return true if your transport is ready for communication
    virtual bool isConnected() = 0;

    /// Will be called by crier whenever it needs to send post serialization data to the transport. (after you call the 'sendMessage' method, for example)
    virtual void sendData(const std::string& data_to_send) = 0;

    /// Will be called by crier on initialization. Crier will use this in order to be able to receive what you deem to be the 'connection was successfully opened' event.
    virtual void setOnConnectCallback(const std::function<void(void)>& on_connect){
      _on_connect_cb = on_connect;
    }

    /// Will be called by crier on initialization. Crier will use this in order to be able to register itself to receive data from your transport.
    virtual void setOnDataCallback(const std::function<void(const std::string&)>& on_data){
      _on_data_cb = on_data;
    }

    /// Will be called by crier on initialization. Crier will use this in order to be able to receive what you deem to be the 'connection was broken' event.
    virtual void setOnDisconnectCallback(const std::function<void(const std::string&)>& on_disconnect){
      _on_disconnect_cb = on_disconnect;
    }

  protected:
    /// Whenever your underlying transport implementation is connected and ready to transmit data, your should invoke this callback.
    std::function<void(void)> _on_connect_cb;

    /// Whenever your receive data from your underlying socket implementation, you should invoke this callback.
    std::function<void(const std::string&)> _on_data_cb;

    /// Whenever your underlying socket implementation has it's connection broken you should invoke this callback, passing it a string that identifies the issue.
    std::function<void(const std::string&)> _on_disconnect_cb;
  };
}

#endif
