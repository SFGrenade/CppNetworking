#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

#include "main.pb.h"
#include "wrapper/reqRepServer.hpp"

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
  void onSimpleRequest( sfpb::SimpleRequest const& reqMsg );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  sfnw::ReqRepServer server_;
  std::thread* thread_;
  bool loop_;
};

}  // namespace SFG

#endif /* SERVER_HPP_ */
