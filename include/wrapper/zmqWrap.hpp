#ifndef ZmqWrap_HPP_
#define ZmqWrap_HPP_

#include <array>
#include <functional>
#include <google/protobuf/message.h>
#include <map>
#include <mutex>
#include <queue>
#include <spdlog/spdlog.h>
#include <string>
#include <zmq.hpp>


namespace SFG {
namespace Networking {

struct Subscription {
  Subscription();
  Subscription( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback );
  google::protobuf::Message* message;
  std::function< void( google::protobuf::Message const& ) > callback;
};

class ZmqWrap {
  public:
  ZmqWrap( std::string const& host, uint16_t port, zmq::socket_type socketType );
  ~ZmqWrap();

  void subscribe( google::protobuf::Message* message, std::function< void( google::protobuf::Message const& ) > callback );
  void sendMessage( google::protobuf::Message* message );

  virtual void run();

  protected:
  virtual bool canSend() const = 0;
  virtual void didSend() = 0;
  virtual bool canRecv() const = 0;
  virtual void didRecv() = 0;

  protected:
  std::shared_ptr< spdlog::logger > logger_;

  std::string host_;
  uint16_t port_;

  zmq::context_t zmqContext_;
  zmq::socket_t zmqSocket_;

  std::mutex mutexForSendQueue_;
  std::queue< google::protobuf::Message* > queueToSend_;
  std::map< std::string, Subscription > subscribedMessages_;
};

}  // namespace Networking
}  // namespace SFG

#endif /* ZmqWrap_HPP_ */
