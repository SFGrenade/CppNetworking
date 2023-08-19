#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <functional>
#include <list>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>
#include <zmqPb/pubSub.hpp>
#include <zmqPb/reqRep.hpp>

#include "main.pb.h"

namespace SFG {

class Server {
  public:
  Server( uint16_t portReqRep, uint16_t portPubSub );
  ~Server();

  void startServer();
  void waitForServer();
  void stopServer();

  private:
  void onMessageRequest( SFG::Proto::MessageRequest const& reqMsg );

  private:
  std::shared_ptr< spdlog::logger > logger_;

  ZmqPb::ReqRep rrNetwork_;
  ZmqPb::PubSub psNetwork_;
  std::thread* thread_;
  bool loop_;

  std::list< std::string > messages_;
};

}  // namespace SFG

#endif /* SERVER_HPP_ */
