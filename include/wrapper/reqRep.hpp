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

#include "wrapper/zmqWrap.hpp"

namespace SFG {
namespace Networking {

class ReqRep : public ZmqWrap {
  public:
  ReqRep( std::string const& host, uint16_t port, bool isServer );
  ~ReqRep();

  virtual void run() override;

  private:
  std::shared_ptr< spdlog::logger > logger_;

  bool isServer_;
};

}  // namespace Networking
}  // namespace SFG

#endif /* REQREP_HPP_ */
