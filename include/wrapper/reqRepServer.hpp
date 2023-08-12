#ifndef REQREPSERVER_HPP_
#define REQREPSERVER_HPP_

#include <functional>
#include <google/protobuf/message.h>
#include <mutex>
#include <queue>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <zmq.hpp>

namespace SFG {
namespace Networking {

class ReqRepServer {
  public:
  ReqRepServer( std::string const& host, uint16_t port );
  ~ReqRepServer();

  void subscribe( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback );
  void sendMessage( google::protobuf::Message* message );

  void run();

  private:
  std::shared_ptr< spdlog::logger > logger_;

  std::string host_;
  uint16_t port_;

  zmq::context_t zmqContext_;
  zmq::socket_t zmqSocket_;

  std::mutex mutexForSendQueue_;
  std::queue< google::protobuf::Message* > queueToSend_;
  std::vector< google::protobuf::Message* > subscribedMessages_;
  std::vector< std::function< void( google::protobuf::Message const& ) > > subscribedCallbacks_;
};

}  // namespace Networking
}  // namespace SFG

#endif /* REQREPSERVER_HPP_ */
