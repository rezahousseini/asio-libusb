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
    , io_executor_(io_ex)
  {  
  }

  void start()
  {
    boost::system::error_code ec;
    asio::post(io_executor_,
    [this, &ec]()
    { 
      do_perform(ec);
      if (ec)
      {
        do_complete(ec);
      }
      else
      {
        if (!peer_.is_open())
        {
          // no device found, search again
          start();
        }
        else
        { 
          do_complete(ec);
        }
      }
    });
  } 

private:

  void do_perform(boost::system::error_code& ec)
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
      if (desc.idVendor == vendor_id_ && desc.idProduct == product_id_)
      {
        peer_.assign(devs[index]);
        peer_.open();
        break;
      } 
    } 
    libusb_free_device_list(devs, 1);
  }

  void do_complete(boost::system::error_code& ec)
  {
    asio::post(io_executor_, 
        [handler = std::move(handler_), &ec](){ handler(ec); });
  } 

private:
  struct libusb_context* context_;
  Device& peer_;
  std::uint16_t vendor_id_;
  std::uint16_t product_id_;
  Handler handler_;
  IoExecutor io_executor_;
};

} // namespace detail
} // namespace libusb
