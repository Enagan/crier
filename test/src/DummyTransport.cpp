#include "DummyTransport.hpp"


void DummyTransport::connect(const std::string& host, int ip) {
  _connected = true;
  _on_connect_cb();
}
void DummyTransport::disconnect() {
  _connected = false;
}
bool DummyTransport::isConnected() {
  return _connected;
}

void DummyTransport::sendData(const std::string& data_to_send) {
  _on_data_cb(data_to_send);
}