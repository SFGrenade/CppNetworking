#ifndef PUBSUB_HPP_
#define PUBSUB_HPP_

#include <array>
#include <functional>
#include <google/protobuf/message.h>
#include <mutex>
#include <queue>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <zmq.hpp>

#include "wrapper/zmqWrap.hpp"

namespace SFG {
namespace Networking {

class PubSub : public ZmqWrap {
  public:
  enum class Status { Receiving, Sending };

  public:
  PubSub( std::string const& host, uint16_t port, bool isServer );
  ~PubSub();

  protected:
  virtual bool canSend() const override;
  virtual void didSend() override;
  virtual bool canRecv() const override;
  virtual void didRecv() override;

  private:
  std::shared_ptr< spdlog::logger > logger_;

  bool isServer_;
  PubSub::Status status_;
};

}  // namespace Networking
}  // namespace SFG

#endif /* PUBSUB_HPP_ */
