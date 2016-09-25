#ifndef TimedEchoTransport_hpp
#define TimedEchoTransport_hpp

#include <string>
#include <functional>

#include "crier/TransportConcept.hpp"

class TimedEchoTransport : public crier::TransportConcept {
public:
  TimedEchoTransport(int time_to_wait);

  void connect(const std::string& host, int ip);
  void disconnect();
  bool isConnected();

  void sendData(const std::string& data_to_send);

private:
  bool _connected = false;
  int _time_to_wait = 0;
};

#endif /* TimedEchoTransport_hpp */
