#include <iostream>
#include "CrierTest.pb.h"

#include "crier/Crier.hpp"

#include "DummyTransport.hpp"

int main(int argc, const char * argv[]) {
  
  crier::Crier<DummyTransport, crier::test::root_msg> net_crier{};
  
  net_crier.registerForTransportOpenedCallback("main.cpp_cb_id", 
    [](){ 
      std::cout << "Connected!!\n"; 
    });
  
  _net_crier.ConnectTransport("dummy_transport_hostname", 1234);
  
  _net_crier.registerPermanentCallback<crier::test::ping>("main.cpp_cb_id", 
    [](const crier::test::ping& ping_msg) {
      std::cout << "Got ping number: " << ping_msg.id() << std::endl;
    });
  
  for (int i = 0; i < 100; i++) {
    crier::test::ping ping_msg;
    ping_msg.set_id(i);
    _net_crier.sendMessage<crier::test::ping>(ping_msg);
  }
}
