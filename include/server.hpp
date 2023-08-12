#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

#include "main.pb.h"
#include "wrapper/reqRepClient.hpp"

namespace SFG {

namespace sfnw = SFG::Networking;
namespace sfpb = SFG::Proto;

class Server {
  public:
  Server( uint16_t port );
  ~Server();

  void startServer();
  void waitForServer();
  void stopServer();

  private:
  void onMessageRequest( sfpb::MessageRequest const& reqMsg );
  void onStopRequest( sfpb::StopRequest const& reqMsg );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  sfnw::ReqRepClient* network_;
  std::thread* thread_;
  bool loop_;
  std::thread* stopThread_;
};

}  // namespace SFG

#endif /* SERVER_HPP_ */
