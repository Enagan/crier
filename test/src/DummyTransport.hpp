#ifndef TransportDummy_hpp
#define TransportDummy_hpp

#include <string>
#include <functional>

#include "crier/Transport.hpp"

class DummyTransport : public crier::Transport {
public:
  void connect(const std::string& host, int ip);
  void disconnect();
  bool isConnected();
  
  void sendData(const std::string& data_to_send);

private:
  bool _connected = false;
};

#endif /* TransportDummy_hpp */
