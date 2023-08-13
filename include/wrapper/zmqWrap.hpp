#ifndef ZmqWrap_HPP_
#define ZmqWrap_HPP_

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

class ZmqWrap {
  public:
  enum class Status { Sending, Receiving };

  public:
  ZmqWrap( std::string const& host, uint16_t port, ZmqWrap::Status startingStatus, zmq::socket_type socketType );
  ~ZmqWrap();

  void subscribe( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback );
  void sendMessage( google::protobuf::Message* message );

  virtual void run() = 0;

  ZmqWrap::Status status() const;
  void setStatus( ZmqWrap::Status newStatus );

  protected:
  std::shared_ptr< spdlog::logger > logger_;

  std::string host_;
  uint16_t port_;
  ZmqWrap::Status status_;

  zmq::context_t zmqContext_;
  zmq::socket_t zmqSocket_;

  std::mutex mutexForSendQueue_;
  std::queue< google::protobuf::Message* > queueToSend_;
  std::vector< google::protobuf::Message* > subscribedMessages_;
  std::vector< std::function< void( google::protobuf::Message const& ) > > subscribedCallbacks_;
};

}  // namespace Networking
}  // namespace SFG

#endif /* ZmqWrap_HPP_ */
