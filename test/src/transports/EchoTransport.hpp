#ifndef EchoTransport_hpp
#define EchoTransport_hpp

#include <string>
#include <functional>

#include "crier/TransportConcept.hpp"

class EchoTransport : public crier::TransportConcept {
public:
  void connect(const std::string& host, int ip);
  void disconnect();
  bool isConnected();

  void sendData(const std::string& data_to_send);

private:
  bool _connected = false;
};

#endif /* EchoTransport_hpp */
