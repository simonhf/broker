#include "broker/detail/network_cache.hh"

namespace broker {
namespace detail {

network_cache::network_cache(caf::event_based_actor* selfptr) : self(selfptr) {
  // nop
}

caf::result<caf::actor> network_cache::fetch(const network_info& x) {
  auto rp = self->make_response_promise();
  fetch(x,
        [=](caf::actor hdl) mutable {
          rp.deliver(std::move(hdl));
        },
        [=](caf::error err) mutable {
          rp.deliver(std::move(err));
        });
  return rp;
}

caf::optional<caf::actor> network_cache::find(const network_info& x) {
  auto i = hdls_.find(x);
  if (i != hdls_.end())
    return i->second;
  return caf::none;
}

caf::optional<network_info> network_cache::find(const caf::actor& x) {
  auto i = addrs_.find(x);
  if (i != addrs_.end())
    return i->second;
  return caf::none;
}

void network_cache::add(const caf::actor& x, const network_info& y) {
  addrs_.emplace(x, y);
  hdls_.emplace(y, x);
}

} // namespace detail
} // namespace broker
