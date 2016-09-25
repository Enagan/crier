#ifndef MessageSendReceiveTests_hpp
#define MessageSendReceiveTests_hpp

#include "protogen/CrierTest.pb.h"
#include "crier/Crier.hpp"
#include "transports/EchoTransport.hpp"
#include "transports/TimedEchoTransport.hpp"

bool TestSimpleSendAndReceiveEcho() {
  bool test_successful = false;
  crier::Crier<EchoTransport, crier::test::root_msg> net_crier{};
  net_crier.connectTransport("localhost", 0); // Echo transport doesn't care

  unsigned int id_to_echo_back = 42;
  crier::test::test_msg_1 msg;
  msg.set_id(id_to_echo_back);
  net_crier.sendMessageWithRetCallback<crier::test::test_msg_1, crier::test::test_msg_1>(msg,
    [&test_successful, &id_to_echo_back](const crier::test::test_msg_1& reply){
      test_successful = reply.id() == id_to_echo_back;
    });

  return test_successful;
}

bool TestSimpleSendAndReceiveEchoBeforeTimeout() {
  bool test_complete = false;
  bool test_successful = false;
  crier::Crier<TimedEchoTransport, crier::test::root_msg> net_crier{TimedEchoTransport(5)};
  net_crier.connectTransport("localhost", 0); // Echo transport doesn't care

  unsigned int id_to_echo_back = 42;
  crier::test::test_msg_1 msg;
  msg.set_id(id_to_echo_back);
  net_crier.sendMessageWithRetCallbackAndTimeout<crier::test::test_msg_1, crier::test::test_msg_1>(msg,
    [&test_successful, &test_complete, &id_to_echo_back](const crier::test::test_msg_1& reply){
      test_complete = true;
      test_successful = reply.id() == id_to_echo_back;
    },
    10,
    [&test_successful, &test_complete](){
      test_complete = true;
      test_successful = false;
    });

  for (size_t times_to_sleep = 3; times_to_sleep > 0; times_to_sleep--) {
    if (test_complete) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
  }
  return test_successful;
}

bool TestSimpleSendAndTimeoutBeforeEcho() {
  bool test_complete = false;
  bool test_successful = false;
  crier::Crier<TimedEchoTransport, crier::test::root_msg> net_crier{TimedEchoTransport(5)};
  net_crier.connectTransport("localhost", 0); // Echo transport doesn't care

  crier::test::test_msg_1 msg;
  msg.set_id(42);
  net_crier.sendMessageWithRetCallbackAndTimeout<crier::test::test_msg_1, crier::test::test_msg_1>(msg,
    [&test_successful, &test_complete](const crier::test::test_msg_1&){
      test_complete = true;
      test_successful = false;
    },
    2,
    [&test_successful, &test_complete](){
      test_complete = true;
      test_successful = true;
    });

  for (size_t times_to_sleep = 3; times_to_sleep > 0; times_to_sleep--) {
    if (test_complete) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
  }
  return test_successful;
}

bool TestMessageSendReceive() {
  return TestSimpleSendAndReceiveEcho() && TestSimpleSendAndReceiveEchoBeforeTimeout() && TestSimpleSendAndTimeoutBeforeEcho();
}

#endif /* MessageSendReceiveTests_hpp */
