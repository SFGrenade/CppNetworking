#ifndef CLIENT_HPP_
#define CLIENT_HPP_

#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

#include "main.pb.h"
#include "wrapper/reqRepClient.hpp"

namespace SFG {

namespace sfnw = SFG::Networking;
namespace sfpb = SFG::Proto;

class Client {
  public:
  Client( std::string const& host, uint16_t port );
  ~Client();

  void startClient();
  void waitForClient();
  void stopClient();

  void sendMessage( std::string const& message );
  void sendStop();

  bool isWaitingForReply() const;
  bool isRunning() const;

  private:
  void onMessageResponse( sfpb::MessageResponse const& repMsg );
  void onStopResponse( sfpb::StopResponse const& repMsg );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  sfnw::ReqRepClient* network_;
  std::thread* thread_;
  bool loop_;
  bool waitingForReply_;
};

}  // namespace SFG

#endif /* CLIENT_HPP_ */
