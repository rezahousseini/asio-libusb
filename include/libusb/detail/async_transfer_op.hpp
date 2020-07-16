#pragma once

#include <boost/asio.hpp>
#include <libusb.h>

namespace asio = boost::asio;

namespace libusb {
namespace detail {

template <typename BufferSequence, typename Handler, typename IoExecutor>
class async_transfer_op : public asio::detail::resolve_op
{
public:
  BOOST_ASIO_DEFINE_HANDLER_PTR(async_transfer_op);

#if defined(BOOST_ASIO_HAS_IOCP)
  typedef class asio::detail::win_iocp_io_context scheduler_impl;
#else
  typedef class asio::detail::scheduler scheduler_impl;
#endif 

  async_transfer_op(struct libusb_context* ctx, 
      struct libusb_device_handle* dev_handle,
      std::uint8_t address, const BufferSequence& buffers, 
      scheduler_impl& sched, Handler& handler, const IoExecutor& io_ex)
    : asio::detail::resolve_op(&async_transfer_op::do_complete)
    , ctx_(ctx)
    , buffers_(buffers)
    , scheduler_(sched)
    , handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler))
    , io_executor_(io_ex) 
    , transfer_(libusb_alloc_transfer(0))
    , transfer_complete_(0)
    , bytes_transferred_(0)
  { 
    libusb_fill_interrupt_transfer(
      transfer_, 
      dev_handle, 
      address, 
      static_cast<unsigned char*>(buffers.data()),
      buffers.size(),
      &callback,
      this,
      0); // 0ms timeout
    asio::detail::handler_work<Handler, IoExecutor>::start(handler_, io_executor_); 
  }

  ~async_transfer_op()
  {
    libusb_free_transfer(transfer_);
  } 

  static void LIBUSB_CALL callback(struct libusb_transfer* transfer)
  {
    auto o(static_cast<async_transfer_op*>(transfer->user_data));

    /* std::cout << "Transfer status: " << transfer->status << std::endl; */
    if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
    {
      o->ec_ = libusb_error(transfer->status);
	  }

    o->bytes_transferred_ = transfer->actual_length;
    o->transfer_complete_ = 1; 
  }

  static void do_complete(void* owner, asio::detail::operation* base, 
      const boost::system::error_code& /*result_ec*/, 
      std::size_t /*bytes_transferred*/)
  { 
    // Take ownership of the operation object.
    auto o(static_cast<async_transfer_op*>(base));
    ptr p = { asio::detail::addressof(o->handler_), o, o };
    asio::detail::handler_work<Handler, IoExecutor> w(o->handler_, o->io_executor_);

    if (owner && owner != &o->scheduler_)
    {
      // The operation is being run on the worker io_context. Time to perform
      // the resolver operation.

      // Perform the blocking transfer operation.
      usb_device_ops::process_transfer(o->ctx_, o->transfer_, 
          &o->transfer_complete_, o->ec_);

      // Pass operation back to main io_context for completion.
      o->scheduler_.post_deferred_completion(o);
      p.v = p.p = 0;
    }
    else
    {
      // The operation has been returned to the main io_context. The completion
      // handler is ready to be delivered. 

      BOOST_ASIO_HANDLER_COMPLETION((*o)); 

      // Make a copy of the handler so that the memory can be deallocated before
      // the upcall is made. Even if we're not about to make an upcall, a
      // sub-object of the handler may be the true owner of the memory associated
      // with the handler. Consequently, a local copy of the handler is required
      // to ensure that any owning sub-object remains valid until after we have
      // deallocated the memory here.
      asio::detail::binder2<Handler, boost::system::error_code, std::size_t>
        handler(o->handler_, o->ec_, o->bytes_transferred_);
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
  } 

private:
  struct libusb_context* ctx_;
  BufferSequence buffers_;
  scheduler_impl& scheduler_;
  Handler handler_;
  IoExecutor io_executor_;  
  struct libusb_transfer* transfer_;
  int transfer_complete_;
  std::size_t bytes_transferred_;
  boost::system::error_code ec_; 
};

} // namespace detail
} // namespace libusb
