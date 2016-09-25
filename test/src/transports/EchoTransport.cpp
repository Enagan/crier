#include "EchoTransport.hpp"

void EchoTransport::connect(const std::string&, int) {
  _connected = true;
  _on_connect_cb();
}
void EchoTransport::disconnect() {
  _connected = false;
  _on_disconnect_cb("User closed transport");
}
bool EchoTransport::isConnected() {
  return _connected;
}
void EchoTransport::sendData(const std::string& data_to_send) {
  _on_data_cb(data_to_send);
}
