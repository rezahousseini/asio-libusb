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
    typedef libusb_device* native_handle_type;
  
    implementation_type()
      : device_(NULL)
      , dev_handle_(NULL)
      , ctx_(NULL)
      , interface_number_(0)
      , endpoint_address_(0)
      , is_handling_events_(false)
    {
    } 
  
  private:
    friend class usb_device_service;
  
    native_handle_type device_;
    struct libusb_device_handle* dev_handle_;
    struct libusb_context* ctx_;
    usb_device_base::interface_number interface_number_;
    usb_device_base::endpoint_address endpoint_address_;
    std::atomic<bool> is_handling_events_;
  };

  typedef implementation_type::native_handle_type native_handle_type;

  explicit usb_device_service(asio::execution_context& context)
    : asio::detail::execution_context_service_base<usb_device_service>(context)
  {
  }

  void construct(implementation_type& impl)
  { 
    if (!impl.ctx_)
    {
      boost::system::error_code ec;
      // use default context for now (instead of &impl.ctx_)
      auto err = libusb_init(NULL);
      ec = libusb_error(err);
      asio::detail::throw_error(ec, "construct");
    }
  }

  void move_construct(implementation_type& impl, 
      implementation_type& other_impl)
  {
    impl.device_ = other_impl.device_;
    other_impl.device_ = NULL;

    impl.dev_handle_ = other_impl.dev_handle_;
    other_impl.dev_handle_ = NULL;

    impl.ctx_ = other_impl.ctx_;
    other_impl.ctx_ = NULL;

    impl.interface_number_ = other_impl.interface_number_;

    impl.endpoint_address_ = other_impl.endpoint_address_;
  }

  void shutdown()
  {
  }

  void destroy(implementation_type& impl)
  {
    boost::system::error_code ignored_ec;
    close(impl, ignored_ec);
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
  void get_option(const implementation_type& impl, 
      GettableUsbDeviceOption& option, boost::system::error_code& ec) const
  {
    do_get_option(impl, option, ec);
  }

  template <typename Device, typename Handler, typename IoExecutor>
  void async_accept(implementation_type& impl, Device& peer, 
      std::uint16_t vendor_id, std::uint16_t product_id, Handler& handler, 
      const IoExecutor& io_ex)
  {
    typedef async_accept_op<Device, Handler, IoExecutor> op;
    typename op::ptr p = { asio::detail::addressof(handler),
      op::ptr::allocate(handler), 0 };
    p.p = new (p.v) op(impl.ctx_, peer, vendor_id, product_id, handler, io_ex);

    BOOST_ASIO_HANDLER_CREATION((context(), *p.p, "device", &impl,
          reinterpret_cast<uintmax_t>(impl.handle_), "async_accept"));

    start_accept_op(impl, p.p);

    p.v = p.p = 0;
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
    typedef async_transfer_op<
      ConstBufferSequence, WriteHandler, IoExecutor> op;
    typename op::ptr p = { asio::detail::addressof(handler),
      op::ptr::allocate(handler), 0 };
    p.p = new (p.v) op(impl.ctx_, buffers, handler, io_ex);

    BOOST_ASIO_HANDLER_CREATION((context(), *p.p, "device", &impl,
          reinterpret_cast<uintmax_t>(impl.handle_), "async_send"));

    start_transfer_op(impl, buffers, impl.endpoint_address_.value(), p.p);

    p.v = p.p = 0;
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
    typedef async_transfer_op<
      MutableBufferSequence, ReadHandler, IoExecutor> op;
    typename op::ptr p = { asio::detail::addressof(handler),
      op::ptr::allocate(handler), 0 };
    p.p = new (p.v) op(impl.ctx_, buffers, handler, io_ex);

    BOOST_ASIO_HANDLER_CREATION((context(), *p.p, "device", &impl,
          reinterpret_cast<uintmax_t>(impl.handle_), "async_receive"));

    start_transfer_op(impl, buffers, impl.endpoint_address_.value() + 128, p.p);

    p.v = p.p = 0;
  } 

private:
  BOOST_ASIO_DECL void do_set_option(implementation_type& impl, 
      const usb_device_base::endpoint_address& option, 
      boost::system::error_code& ec);

  BOOST_ASIO_DECL void do_set_option(implementation_type& impl, 
      const usb_device_base::interface_number& option, 
      boost::system::error_code& ec);

  BOOST_ASIO_DECL void do_get_option(const implementation_type& impl, 
      usb_device_base::endpoint_address& option, 
      boost::system::error_code& ec) const;

  BOOST_ASIO_DECL void do_get_option(const implementation_type& impl, 
      usb_device_base::interface_number& option, 
      boost::system::error_code& ec) const;

  template <typename Operation>
  BOOST_ASIO_DECL void start_accept_op(implementation_type& impl,
      Operation* op);

  template <typename BufferSequence, typename Operation>
  BOOST_ASIO_DECL void start_transfer_op(implementation_type& impl, 
      const BufferSequence& buffers, std::uint8_t address, Operation* op);
};

} // namespace detail
} // namespace libusb

#include "libusb/detail/impl/usb_device_service.ipp"
