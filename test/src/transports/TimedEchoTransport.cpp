#include <thread>

#include "TimedEchoTransport.hpp"

TimedEchoTransport::TimedEchoTransport(int time_to_wait) : _time_to_wait(time_to_wait) {}

void TimedEchoTransport::connect(const std::string&, int) {
  _connected = true;
  _on_connect_cb();
}
void TimedEchoTransport::disconnect() {
  _connected = false;
  _on_disconnect_cb("User closed transport");
}
bool TimedEchoTransport::isConnected() {
  return _connected;
}
void TimedEchoTransport::sendData(const std::string& data_to_send) {
  std::thread timed_echo([this, data_to_send](){
    std::this_thread::sleep_for(std::chrono::milliseconds{this->_time_to_wait});
    _on_data_cb(data_to_send);
  });
  timed_echo.detach();
}
