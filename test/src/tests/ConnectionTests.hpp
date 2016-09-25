#ifndef ConnectionTests_hpp
#define ConnectionTests_hpp

#include "protogen/CrierTest.pb.h"
#include "crier/Crier.hpp"
#include "transports/EchoTransport.hpp"

bool TestSimpleConnect() {
  bool test_successfull = false;
  crier::Crier<EchoTransport, crier::test::root_msg> net_crier{};
  net_crier.registerForTransportOpenedCallback("TestSimpleConnect",
    [&test_successfull](){
      test_successfull = true;
    });
  net_crier.connectTransport("localhost", 0); // Echo transport doesn't care
  return net_crier.transportConnected() && test_successfull;
}

bool TestSimpleDisconnect() {
  bool test_successfull = false;
  crier::Crier<EchoTransport, crier::test::root_msg> net_crier{};
  net_crier.registerForTransportClosedCallback("TestSimpleDisconnect",
  [&test_successfull](const std::string& reason){
    test_successfull = reason == "User closed transport";
  });
  net_crier.connectTransport("localhost", 0); // Echo transport doesn't care

  if (net_crier.transportConnected()) {
    net_crier.disconnectTransport();
  }

  return !net_crier.transportConnected() && test_successfull;
}

bool TestCrierTransportConnection() {
  return TestSimpleConnect() && TestSimpleDisconnect();
}

#endif /* ConnectionTests_hpp */
