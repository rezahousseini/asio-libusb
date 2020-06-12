#pragma once

#include <libusb.h>

namespace asio = boost::asio;

namespace libusb {
namespace detail {

template <typename MutableBufferSequence, typename Handler, 
         typename IoExecutor>
class async_transfer_op
{
public:
  async_transfer_op(libusb_device_handle* dev_handle,
                    std::uint8_t endpoint,
                    const MutableBufferSequence& buffers,
                    Handler& handler, const IoExecutor& io_ex)
    : buffers_(buffers)
    , handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler))
    , io_executor_(io_ex)
    , transfer_(libusb_alloc_transfer(0))
  { 
    asio::detail::buffer_sequence_adapter<asio::mutable_buffer,
      MutableBufferSequence> bufs(buffers_);

    async_transfer_op *op = new async_transfer_op(
      BOOST_ASIO_MOVE_CAST(async_transfer_op)(*this));

    libusb_fill_interrupt_transfer(transfer_, 
      dev_handle, 
      endpoint, 
      bufs.buffers(),
      bufs.count(),
      &callback,
      op,
      0);
  }

  ~async_transfer_op()
  {
    libusb_free_transfer(transfer_);
  }

  void start()
  {
    int err = libusb_submit_transfer(transfer_);
    if (err != LIBUSB_SUCCESS)
    {
      auto ec = error(libusb_error(err)).error_code();
      do_complete(ec, 0);
    }
  } 

  void callback(struct libusb_transfer* transfer)
  {
    auto op = std::make_unique<async_transfer_op>(
      static_cast<async_transfer_op*>(transfer->user_data));
    bool expected(false);

    // call handler
    auto ec = error(libusb_error(transfer->status)).error_code();
    op->do_complete(ec, transfer->actual_length);
  } 

private:

  void do_complete(boost::system::error_code& ec, 
      std::size_t bytes_transferred)
  {
    io_executor_.post(
      [handler = std::move(handler_), &ec, bytes_transferred]()
      { 
        handler(ec, bytes_transferred);
      });
  } 

private:
  MutableBufferSequence buffers_;
  Handler handler_;
  IoExecutor io_executor_;
  struct libusb_transfer* transfer_;
};

} // namespace detail
} // namespace libusb
