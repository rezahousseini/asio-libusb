#pragma once

#include <boost/asio.hpp>
#include "libusb/usb_device.hpp"
#include "libusb/usb_device_base.hpp"
#include "libusb/detail/usb_device_service.hpp"

namespace libusb {

namespace asio = boost::asio;

template <typename Executor = asio::executor>
class usb_device_acceptor
  : public usb_device_base
{
public:
  /// The type of the executor associated with the object.
  typedef Executor executor_type;

  /// Rebinds the acceptor type to another executor.
  template <typename Executor1>
  struct rebind_executor
  {
    /// The socket type when rebound to the specified executor.
    typedef usb_device_acceptor<Executor1> other;
  };

  /// The native representation of an acceptor.
  typedef typename detail::usb_device_service::native_handle_type 
    native_handle_type;

  /// Construct an acceptor without opening it.
  /**
   * This constructor creates an acceptor without opening it to listen for new
   * connections. The open() function must be called before the acceptor can
   * accept new usb device connections.
   *
   * @param ex The I/O executor that the acceptor will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * acceptor.
   */
  explicit usb_device_acceptor(const executor_type& ex)
    : impl_(ex)
  {
  }

  /// Construct an acceptor without opening it.
  /**
   * This constructor creates an acceptor without opening it to listen for new
   * connections. The open() function must be called before the acceptor can
   * accept new socket connections.
   *
   * @param context An execution context which provides the I/O executor that
   * the acceptor will use, by default, to dispatch handlers for any
   * asynchronous operations performed on the acceptor.
   */
  template <typename ExecutionContext>
  explicit usb_device_acceptor(ExecutionContext& context,
      typename std::enable_if<
        asio::is_convertible<ExecutionContext&, 
        asio::execution_context&>::value>::type* = 0)
    : impl_(context)
  {
  }

  /// Start an asynchronous accept.
  /**
   * This function is used to asynchronously accept a new connection into a
   * usb device. The function call always returns immediately.
   *
   * @param peer The usb device into which the new connection will be accepted.
   * Ownership of the peer object is retained by the caller, which must
   * guarantee that it is valid until the handler is called.
   *
   * @param vendor_id The vendor id of the accepting device.
   *
   * @param product_id The product id of the accepting device.
   *
   * @param handler The handler to be called when the accept operation
   * completes. Copies will be made of the handler as required. The function
   * signature of the handler must be:
   * @code void handler(
   *   const boost::system::error_code& error // Result of operation.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the handler will not be invoked from within this function. On
   * immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using asio::post().
   */
 template <typename Executor1, typename AcceptHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(AcceptHandler,
      void (boost::system::error_code))
  async_accept(usb_device<Executor1>& peer, std::uint16_t vendor_id, 
      std::uint16_t product_id, BOOST_ASIO_MOVE_ARG(AcceptHandler) handler)
  {
    return asio::async_initiate<AcceptHandler,
      void (boost::system::error_code)>(
        initiate_async_accept(), handler, this, &peer, vendor_id, product_id);
  }

private:
  // Disallow copying and assignment.
  usb_device_acceptor(const usb_device_acceptor&) BOOST_ASIO_DELETED;
  usb_device_acceptor& operator=(const usb_device_acceptor&) BOOST_ASIO_DELETED;

  struct initiate_async_accept
  {
    template <typename AcceptHandler, typename Executor1>
    void operator()(BOOST_ASIO_MOVE_ARG(AcceptHandler) handler,
        usb_device_acceptor* self, usb_device<Executor1>* peer,
        std::uint16_t vendor_id, std::uint16_t product_id) const
    {
      // If you get an error on the following line it means that your handler
      // does not meet the documented type requirements for a AcceptHandler.
      BOOST_ASIO_ACCEPT_HANDLER_CHECK(AcceptHandler, handler) type_check;

      asio::detail::non_const_lvalue<AcceptHandler> handler2(handler);
      self->impl_.get_service().async_accept(
          self->impl_.get_implementation(), *peer, vendor_id, product_id,
          handler2.value, self->impl_.get_implementation_executor());
    }
  };

  asio::detail::io_object_impl<detail::usb_device_service, Executor> impl_;
};

} // namespace libusb
