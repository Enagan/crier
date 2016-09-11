#ifndef CRIER_TRANSPORT_HPP
#define CRIER_TRANSPORT_HPP

#include <string>
#include <functional>

namespace crier {
  class Transport {
  public:
    virtual void connect(const std::string& host, int ip) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() = 0;
    
    virtual void sendData(const std::string& data_to_send) = 0;
    
    virtual void setOnConnectCallback(const std::function<void(void)>& on_connect){
      _on_connect_cb = on_connect;
    }
    virtual void setOnDataCallback(const std::function<void(const std::string&)>& on_data){
      _on_data_cb = on_data;
    }
    virtual void setOnDisconnectCallback(const std::function<void(const std::string&)>& on_disconnect){
      _on_disconnect_cb = on_disconnect;
    }
    
  protected:
    std::function<void(void)> _on_connect_cb;
    std::function<void(const std::string&)> _on_data_cb;
    std::function<void(const std::string&)> _on_disconnect_cb;
  };
}

#endif