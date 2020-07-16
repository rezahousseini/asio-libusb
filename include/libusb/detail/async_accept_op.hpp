#pragma once

#include <libusb.h>
#include "usb_device_ops.hpp"

namespace asio = boost::asio;

namespace libusb {
namespace detail {

template <typename Device, typename Handler, typename IoExecutor>
class async_accept_op : public asio::detail::resolve_op
{
public:
  BOOST_ASIO_DEFINE_HANDLER_PTR(async_accept_op);

#if defined(BOOST_ASIO_HAS_IOCP)
  typedef class asio::detail::win_iocp_io_context scheduler_impl;
#else
  typedef class asio::detail::scheduler scheduler_impl;
#endif

  async_accept_op(struct libusb_context* ctx, Device& peer, 
      std::uint16_t vendor_id, std::uint16_t product_id, scheduler_impl& sched, 
      Handler& handler, const IoExecutor& io_ex)
    : asio::detail::resolve_op(&async_accept_op::do_complete)
    , ctx_(ctx)
    , peer_(peer)
    , vendor_id_(vendor_id)
    , product_id_(product_id)
    , scheduler_(sched)
    , handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler))
    , io_executor_(io_ex)
  { 
    asio::detail::handler_work<Handler, IoExecutor>::start(handler_, io_executor_);
  } 

  void do_assign()
  {
    peer_.assign(new_device_, ec_);
  } 

  static void do_complete(void* owner, asio::detail::operation* base, 
      const boost::system::error_code& /*result_ec*/, 
      std::size_t /*bytes_transferred*/)
  { 
    auto o(static_cast<async_accept_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };
    asio::detail::handler_work<Handler, IoExecutor> w(o->handler_, o->io_executor_);

    if (owner && owner != &o->scheduler_)
    {
      // The operation is being run on the worker io_context. Time to perform
      // the resolver operation.

      // Perform the blocking find operation.
      bool success = usb_device_ops::find_device(o->ctx_, &o->new_device_, 
        o->vendor_id_, o->product_id_, o->ec_);

      if (success)
      {
        // Pass operation back to main io_context for completion.
        o->scheduler_.post_deferred_completion(o);
      }
      p.v = p.p = 0;
    }
    else
    {
      // On success, assign new device to peer device object
      if (owner)
        o->do_assign();

      BOOST_ASIO_HANDLER_COMPLETION((*o));

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made. Even if we're not about to make an upcall, a
      // sub-object of the handler may be the true owner of the memory associated
      // with the handler. Consequently, a local copy of the handler is required
      // to ensure that any owning sub-object remains valid until after we have
      // deallocated the memory here.
      asio::detail::binder1<Handler, boost::system::error_code> 
        handler(o->handler_, o->ec_);
      p.h = asio::detail::addressof(handler.handler_);
      p.reset();

      // Make the upcall if required.
      if (owner)
      {
        asio::detail::fenced_block b(asio::detail::fenced_block::half);
        BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
        w.complete(handler, handler.handler_);
        BOOST_ASIO_HANDLER_INVOCATION_END;
      }
    }
  } 

private:
  struct libusb_context* ctx_;
  Device& peer_;
  std::uint16_t vendor_id_;
  std::uint16_t product_id_;
  scheduler_impl& scheduler_;
  Handler handler_;
  IoExecutor io_executor_;
  struct libusb_device* new_device_;
  boost::system::error_code ec_;
};

} // namespace detail
} // namespace libusb
