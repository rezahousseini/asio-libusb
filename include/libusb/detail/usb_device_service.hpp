#pragma once

#include <boost/asio.hpp>
#include <libusb.h>
#include "libusb/usb_device_base.hpp"
#include "libusb/error.hpp"
#include "libusb/detail/async_accept_op.hpp"
#include "libusb/detail/async_transfer_op.hpp"

namespace libusb {
namespace detail {

namespace asio = boost::asio;

class usb_device_service
 : public asio::detail::execution_context_service_base<usb_device_service>
{
public:  
  class implementation_type
  {
  public:
    typedef struct libusb_device* native_handle_type;
  
    implementation_type()
      : device_(NULL)
      , dev_handle_(NULL)
      , ctx_()
      , interface_number_(0)
      , endpoint_address_(0)
      , do_handle_events_(false)
    {
    } 
  
    template <typename IoExecutor>
    void handle_events(const IoExecutor& io_ex)
    {
      if (!do_handle_events_)
      {
        do_handle_events_ = true;
        std::cout << "Start handling events" << std::endl;
        asio::post(io_ex, 
        [this]()
        { 
          while(do_handle_events_)
            libusb_handle_events(NULL);
        }); 
      }
    }
  
  private:
    friend class usb_device_service;
  
    native_handle_type device_;
    struct libusb_device_handle* dev_handle_;
    struct libusb_context* ctx_;
    usb_device_base::interface_number interface_number_;
    usb_device_base::endpoint_address endpoint_address_;
    std::atomic<bool> do_handle_events_;
  };

  typedef implementation_type::native_handle_type native_handle_type;

  explicit usb_device_service(asio::execution_context& context)
    : asio::detail::execution_context_service_base<usb_device_service>(context)
  {
  }

  void construct(implementation_type& impl)
  {
    boost::system::error_code ec;
    auto err = libusb_init(&impl.ctx_);
    ec = error(libusb_error(err)).error_code();
    asio::detail::throw_error(ec, "construct");
  }

  void shutdown()
  {
  }

  void destroy(implementation_type& impl)
  {
    if (is_open(impl))
    {
      boost::system::error_code ignored_ec;
      close(impl, ignored_ec);
    }
    libusb_exit(impl.ctx_);
  }

  BOOST_ASIO_DECL void open(implementation_type& impl,
      boost::system::error_code& ec);

  BOOST_ASIO_DECL void assign(implementation_type& impl, 
      native_handle_type native_usb_device, boost::system::error_code& ec);

  BOOST_ASIO_DECL bool is_open(const implementation_type& impl) const;

  BOOST_ASIO_DECL void close(implementation_type& impl, 
      boost::system::error_code& ec);

  BOOST_ASIO_DECL native_handle_type native_handle(implementation_type& impl);

  BOOST_ASIO_DECL void cancel(implementation_type& impl, 
      boost::system::error_code& ec);

  template <typename SettableUsbDeviceOption>
  void set_option(implementation_type& impl, 
      const SettableUsbDeviceOption& option, boost::system::error_code& ec)
  {
    do_set_option(impl, option, ec);
  }

  template <typename GettableUsbDeviceOption>
  void get_option(implementation_type& impl, 
      GettableUsbDeviceOption& option, boost::system::error_code& ec)
  {
    do_get_option(impl, option, ec);
  }

  template <typename Device, typename Handler, typename IoExecutor>
  void async_accept(implementation_type& impl, Device& peer, 
      std::uint16_t vendor_id, std::uint16_t product_id, Handler& handler, 
      const IoExecutor& io_ex)
  {
    async_accept_op<Device, Handler, IoExecutor> op(impl.ctx_, peer, 
        vendor_id, product_id, handler, io_ex);

      op.start();
  }

  template <typename ConstBufferSequence>
  BOOST_ASIO_DECL std::size_t send(implementation_type& impl, 
    const ConstBufferSequence& buffers, boost::system::error_code& ec);

  template <typename WriteHandler, typename ConstBufferSequence, 
           typename IoExecutor>
  void async_send(implementation_type& impl, 
      const ConstBufferSequence& buffers,
      WriteHandler& handler, const IoExecutor& io_ex)
  {
    // start libusb event handling
    impl.handle_events(io_ex);

    async_transfer_op<ConstBufferSequence, WriteHandler, IoExecutor> op(
        impl.dev_handle_, impl.endpoint_address_.value(), buffers, handler, 
        io_ex);

    op.start();
  }

  template <typename MutableBufferSequence>
  BOOST_ASIO_DECL std::size_t receive(implementation_type& impl, 
    const MutableBufferSequence& buffers, boost::system::error_code& ec);

  template <typename ReadHandler, typename MutableBufferSequence, 
           typename IoExecutor>
  void async_receive(implementation_type& impl, 
      const MutableBufferSequence& buffers,
      ReadHandler& handler, const IoExecutor& io_ex)
  {
    // start libusb event handling
    impl.handle_events(io_ex);

    async_transfer_op<MutableBufferSequence, ReadHandler, IoExecutor> op(
        impl.dev_handle_, impl.endpoint_address_.value(), buffers, handler, 
        io_ex);

    op.start();
  } 

private:
  BOOST_ASIO_DECL void do_set_option(implementation_type& impl, 
      const usb_device_base::endpoint_address& option, 
      boost::system::error_code& ec);

  BOOST_ASIO_DECL void do_set_option(implementation_type& impl, 
      const usb_device_base::interface_number& option, 
      boost::system::error_code& ec);

  BOOST_ASIO_DECL void do_get_option(implementation_type& impl, 
      usb_device_base::endpoint_address& option, 
      boost::system::error_code& ec);

  BOOST_ASIO_DECL void do_get_option(implementation_type& impl, 
      usb_device_base::interface_number& option, 
      boost::system::error_code& ec);

};

} // namespace detail
} // namespace libusb

#include "libusb/detail/impl/usb_device_service.ipp"
