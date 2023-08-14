#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

#include "main.pb.h"
#include "wrapper/reqRep.hpp"

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
  bool isRunning() const;

  private:
  void onMessageResponse( SFG::Proto::MessageResponse const& repMsg );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  SFG::Networking::ReqRep network_;
  std::thread* thread_;
  bool loop_;
  bool waitingForMessageResponse_;
};

}  // namespace SFG

#endif /* CLIENT_HPP_ */
