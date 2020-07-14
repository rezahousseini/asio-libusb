#pragma once

#include <boost/asio.hpp>
#include <libusb.h>

namespace asio = boost::asio;

namespace libusb {
namespace detail {

template <typename BufferSequence, typename Handler, typename IoExecutor>
class async_transfer_op
{
public:
  BOOST_ASIO_DEFINE_HANDLER_PTR(async_transfer_op);

  struct libusb_transfer* transfer;

  async_transfer_op(struct libusb_context* ctx, const BufferSequence& buffers, 
      Handler& handler, const IoExecutor& io_ex)
    : ctx_(ctx)
    , buffers_(buffers)
    , handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler))
    , io_executor_(io_ex)
    , transfer_complete_(0)
    , bytes_transferred_(0)
  { 
    transfer = libusb_alloc_transfer(0);
    asio::detail::handler_work<Handler, IoExecutor>::start(handler_, io_executor_); 
  }

  template <typename Operator>
  static bool do_perform(Operator* o)
  {
    /* std::cout << "perform" << std::endl; */ 
    int err = libusb_handle_events_completed(o->ctx_, &o->transfer_complete_);
    /* std::cout << "end handle event with: " << err << std::endl; */
    o->ec_ = libusb_error(err);
    return err == LIBUSB_SUCCESS and !o->transfer_complete_;
  }

  static void LIBUSB_CALL callback(struct libusb_transfer* transfer)
  {
    auto o = static_cast<
      async_transfer_op<BufferSequence, Handler, IoExecutor>*>(
          transfer->user_data);

    /* std::cout << "Transfer status: " << transfer->status << std::endl; */
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
    {
      o->ec_ = libusb_error(transfer->status);
	  }

    o->bytes_transferred_ = transfer->actual_length;
    o->transfer_complete_ = 1;

    libusb_free_transfer(transfer);
  }

  template <typename Operation>
  static void do_complete(void* owner, Operation* o)
  {
    do_complete(owner, o, o->ec_, o->bytes_transferred_);
  }

  template <typename Operation>
  static void do_complete(void* owner, Operation* o, 
      const boost::system::error_code& ec, std::size_t bytes_transferred)
  { 
    ptr p = { asio::detail::addressof(o->handler_), o, o };

    asio::detail::handler_work<Handler, IoExecutor> w(o->handler_, o->io_executor_);

    BOOST_ASIO_HANDLER_COMPLETION((*o));

    // Make a copy of the handler so that the memory can be deallocated before
    // the upcall is made. Even if we're not about to make an upcall, a
    // sub-object of the handler may be the true owner of the memory associated
    // with the handler. Consequently, a local copy of the handler is required
    // to ensure that any owning sub-object remains valid until after we have
    // deallocated the memory here.
    asio::detail::binder2<Handler, boost::system::error_code, std::size_t>
      handler(o->handler_, ec, bytes_transferred);
    p.h = asio::detail::addressof(handler.handler_);
    p.reset();

    // Make the upcall if required.
    if (owner)
    {
      asio::detail::fenced_block b(asio::detail::fenced_block::half);
      BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
      w.complete(handler, handler.handler_);
      BOOST_ASIO_HANDLER_INVOCATION_END;
    }
  } 

private:
  struct libusb_context* ctx_;
  BufferSequence buffers_;
  Handler handler_;
  IoExecutor io_executor_;  
  int transfer_complete_;
  std::size_t bytes_transferred_;
  boost::system::error_code ec_; 
};

} // namespace detail
} // namespace libusb
