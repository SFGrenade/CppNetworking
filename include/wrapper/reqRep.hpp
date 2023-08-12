#ifndef REQREP_HPP_
#define REQREP_HPP_

#include <array>
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

class ReqRep {
  public:
  ReqRep( std::string const& host, uint16_t port, bool isServer );
  ~ReqRep();

  void subscribe( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback );
  void sendMessage( google::protobuf::Message* message );

  void run();

  private:
  std::shared_ptr< spdlog::logger > logger_;

  std::string host_;
  uint16_t port_;
  bool isServer_;

  zmq::context_t zmqContext_;
  zmq::socket_t zmqSocket_;

  std::mutex mutexForSendQueue_;
  std::queue< google::protobuf::Message* > queueToSend_;
  std::vector< google::protobuf::Message* > subscribedMessages_;
  std::vector< std::function< void( google::protobuf::Message const& ) > > subscribedCallbacks_;
  bool sending_;
};

}  // namespace Networking
}  // namespace SFG

#endif /* REQREP_HPP_ */
