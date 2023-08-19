#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <zmqPb/pubSub.hpp>
#include <zmqPb/reqRep.hpp>

#include "main.pb.h"

namespace SFG {

class Client {
  public:
  Client( std::string const& hostReqRep, uint16_t portReqRep, std::string const& hostPubSub, uint16_t portPubSub );
  ~Client();

  void startClient();
  void waitForClient();
  void stopClient();

  void sendMessage( std::string const& message );

  bool isWaitingForReply() const;
  bool isRunning() const;

  private:
  void onMessageResponse( SFG::Proto::MessageResponse const& repMsg );
  void onAllMessages( SFG::Proto::AllMessages const& msg );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  ZmqPb::ReqRep rrNetwork_;
  ZmqPb::PubSub psNetwork_;
  std::thread* thread_;
  bool loop_;
  bool waitingForMessageResponse_;
};

}  // namespace SFG

#endif /* CLIENT_HPP_ */
