#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

#include "main.pb.h"
#include "reqRepClient.hpp"

namespace SFG {

class Client {
  public:
  Client( std::string const& host, uint16_t port );
  ~Client();

  void startClient();
  void waitForClient();
  void stopClient();

  void sendMessage( std::string const& message );

  bool isWaitingForReply() const;

  private:
  void onSimpleResponse( SFG::Proto::SimpleResponse const& repMsg );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  ZmqPbWrap::ReqRepClient client_;
  std::thread* thread_;
  bool loop_;
  bool waitingForReply_;
};

}  // namespace SFG

#endif /* CLIENT_HPP_ */
