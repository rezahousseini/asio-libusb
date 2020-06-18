#pragma once

#include <libusb.h>

namespace asio = boost::asio;

namespace libusb {
namespace detail {

template <typename Device, typename Handler, typename IoExecutor>
class async_accept_op
{
public:
  async_accept_op(struct libusb_context* context,
                  Device& peer, std::uint16_t vendor_id, 
                  std::uint16_t product_id, Handler& handler, 
                  const IoExecutor& io_ex)
    : context_(context)
    , peer_(peer)
    , vendor_id_(vendor_id)
    , product_id_(product_id)
    , handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler))
    , work_(asio::make_work_guard(io_ex))
  { 
  } 

  void start()
  {
    asio::post(work_.get_executor(),
    [this, handler=std::move(handler_)]() mutable
    { 
      boost::system::error_code ec;
      while(!ec and !peer_.is_open())
      {
        find_device(std::move(peer_), vendor_id_, product_id_, ec);
      }
      asio::dispatch(work_.get_executor(), 
        [handler=std::move(handler), ec=std::move(ec)]() mutable 
        { 
          handler(std::move(ec)); 
        });
    });
  }

private:

  void find_device(Device&& peer, std::uint16_t vendor_id, 
      std::uint16_t product_id, boost::system::error_code& ec)
  {
    libusb_device** devs;
    
    int cnt = libusb_get_device_list(context_, &devs);
    if (cnt < 0)
    {
      ec = error(libusb_error(cnt)).error_code();
      libusb_free_device_list(devs, 1);
	    return;
    }

    int index;
    for (index = 0; index < cnt; index++)
    {
      struct libusb_device_descriptor desc;

      auto ret = libusb_get_device_descriptor(devs[index], &desc);
      if (ret < 0)
      {
        ec = error(libusb_error(ret)).error_code();
	      break;
      }

      if (desc.idVendor == vendor_id && desc.idProduct == product_id)
      {
        peer.assign(devs[index], ec);
        if (ec)
          break;
        peer.open(ec);
        break;
      } 
    } 
    libusb_free_device_list(devs, 1);
  }

private:
  struct libusb_context* context_;
  Device& peer_;
  std::uint16_t vendor_id_;
  std::uint16_t product_id_;
  Handler handler_;
  asio::executor_work_guard<IoExecutor> work_;
};

} // namespace detail
} // namespace libusb
