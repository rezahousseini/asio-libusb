#pragma once

#include <libusb.h>

namespace asio = boost::asio;

namespace libusb {
namespace detail {

template <typename Device, typename Handler, typename IoExecutor>
class async_accept_op
{
public:
  BOOST_ASIO_DEFINE_HANDLER_PTR(async_accept_op);

  async_accept_op(struct libusb_context* ctx, Device& peer, 
      std::uint16_t vendor_id, std::uint16_t product_id, Handler& handler, 
      const IoExecutor& io_ex)
    : ctx_(ctx)
    , peer_(peer)
    , vendor_id_(vendor_id)
    , product_id_(product_id)
    , handler_(BOOST_ASIO_MOVE_CAST(Handler)(handler))
    , io_executor_(io_ex)
  { 
    asio::detail::handler_work<Handler, IoExecutor>::start(handler_, io_executor_);
  } 

  template <typename Operation>
  static bool do_perform(Operation* o)
  {
    struct libusb_device* new_device = nullptr;
    bool result = find_device(o->ctx_, &new_device, o->vendor_id_, 
        o->product_id_, o->ec_);
    o->new_device_ = new_device;
    return result;
  }

  void do_assign()
  {
    peer_.assign(new_device_, ec_);
  }

  template <typename Operation>
  static void do_complete(void* owner, Operation* o)
  {
    do_complete(owner, o, o->ec_);
  }

  template <typename Operation>
  static void do_complete(void* owner, Operation* o, 
      const boost::system::error_code& ec)
  { 
    ptr p = { asio::detail::addressof(o->handler_), o, o };

    asio::detail::handler_work<Handler, IoExecutor> w(o->handler_, o->io_executor_);

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
      handler(o->handler_, ec);
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

private:
  static bool find_device(struct libusb_context* ctx, 
      struct libusb_device** peer, std::uint16_t vendor_id, 
      std::uint16_t product_id, boost::system::error_code& ec)
  {
    bool success = false;
    libusb_device** devs;
    
    int cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0)
    {
      ec = libusb_error(cnt);
      libusb_free_device_list(devs, 1);
      return success;
    }
  
    int index;
    for (index = 0; index < cnt; index++)
    {
      struct libusb_device_descriptor desc;
  
      auto ret = libusb_get_device_descriptor(devs[index], &desc);
      if (ret < 0)
      {
        ec = libusb_error(ret);
        break;
      }
  
      if (desc.idVendor == vendor_id && desc.idProduct == product_id)
      {
        *peer = libusb_ref_device(devs[index]);
        success = true;
        break;
      } 
    } 
    libusb_free_device_list(devs, 1);
    return success;
  }

private:
  struct libusb_context* ctx_;
  Device& peer_;
  std::uint16_t vendor_id_;
  std::uint16_t product_id_;
  Handler handler_;
  IoExecutor io_executor_;
  struct libusb_device* new_device_;
  boost::system::error_code ec_;
};

} // namespace detail
} // namespace libusb
