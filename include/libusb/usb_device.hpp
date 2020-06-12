#pragma once

#include <string>
#include <boost/asio.hpp>
#include "libusb/usb_device_base.hpp"
#include "libusb/detail/usb_device_service.hpp"

namespace libusb {

namespace asio = boost::asio;

template <typename Executor = asio::executor>
class usb_device
  : public usb_device_base
{
public:

  /// The type of the executor associated with the object.
  typedef Executor executor_type;

  /// Rebinds the usb device type to another executor.
  template <typename Executor1>
  struct rebind_executor
  {
    /// The usb device type when rebound to the specified executor.
    typedef usb_device<Executor1> other;
  };

  /// The native representation of a usb device.
  typedef detail::usb_device_service::native_handle_type native_handle_type;

  /// A usb_device is always the lowest layer.
  typedef usb_device lowest_layer_type;

  /// Construct a usb_device without opening it.
  /**
   * This constructor creates a usb device without opening it.
   *
   * @param ex The I/O executor that the usb device will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * usb device.
   */
  explicit usb_device(const executor_type& ex)
    : impl_(ex)
  { 
  }

  /// Construct and open an usb_device.
  /**
   * This constructor creates and opens an usb device for the specified vendor 
   * and product id.
   *
   * @param ex The I/O executor that the usb device will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * usb device.
   *
   * @param vendor_id The vendor id for this usb device.
   * @param product_id The product id for this usb device.
   */
  usb_device(const executor_type& ex, std::uint16_t vendor_id, 
      std::uint16_t product_id)
    : impl_(ex)
  {
    boost::system::error_code ec;
    impl_.get_service().open(impl_.get_implementation(), 
        vendor_id, product_id, ec, impl_.get_implementation_executor());
    asio::detail::throw_error(ec, "open");
  }

  /// Construct a usb_device on an existing native usb device.
  /**
   * This constructor creates a usb device object to hold an existing native
   * usb device.
   *
   * @param ex The I/O executor that the usb device will use, by default, to
   * dispatch handlers for any asynchronous operations performed on the
   * usb device.
   *
   * @param native_usb_device A native usb device.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  usb_device(const executor_type& ex,
      const native_handle_type& native_usb_device)
    : impl_(ex)
  {
    boost::system::error_code ec;
    impl_.get_service().assign(impl_.get_implementation(),
        native_usb_device, ec);
    asio::detail::throw_error(ec, "assign");
  }

  /// Move-construct a usb_device from another.
  /**
   * This constructor moves a usb device from one object to another.
   *
   * @param other The other usb_device object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c usb_device(const executor_type&)
   * constructor.
   */
  usb_device(usb_device&& other)
    : impl_(std::move(other.impl_))
  {
  }

  /// Move-assign a usb_device from another.
  /**
   * This assignment operator moves a usb device from one object to another.
   *
   * @param other The other usb_device object from which the move will
   * occur.
   *
   * @note Following the move, the moved-from object is in the same state as if
   * constructed using the @c usb_device(const executor_type&)
   * constructor.
   */
  usb_device& operator=(usb_device&& other)
  {
    impl_ = std::move(other.impl_);
    return *this;
  }

  /// Destroys the usb device.
  /**
   * This function destroys the usb device, cancelling any outstanding
   * asynchronous wait operations associated with the usb device as if by
   * calling @c cancel.
   */
  ~usb_device()
  {
  }

  /// Get the executor associated with the object.
  executor_type get_executor() BOOST_ASIO_NOEXCEPT
  {
    return impl_.get_executor();
  }

  /// Get a reference to the lowest layer.
  /**
   * This function returns a reference to the lowest layer in a stack of
   * layers. Since a usb_device cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A reference to the lowest layer in the stack of layers. Ownership
   * is not transferred to the caller.
   */
  lowest_layer_type& lowest_layer()
  {
    return *this;
  }

  /// Get a const reference to the lowest layer.
  /**
   * This function returns a const reference to the lowest layer in a stack of
   * layers. Since a usb_device cannot contain any further layers, it
   * simply returns a reference to itself.
   *
   * @return A const reference to the lowest layer in the stack of layers.
   * Ownership is not transferred to the caller.
   */
  const lowest_layer_type& lowest_layer() const
  {
    return *this;
  }

  /// Open the usb device using the specified vendor and product id.
  /**
   * This function opens the usb device for the specified vendor and 
   * product id.
   *
   * @param vendor_id The vendor id for this usb device.
   * @param product_id The product id for this usb device.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void open(std::uint16_t vendor_id, std::uint16_t product_id)
  {
    boost::system::error_code ec;
    impl_.get_service().open(impl_.get_implementation(), vendor_id, 
        product_id, ec, impl_.get_implementation_executor());
    asio::detail::throw_error(ec, "open");
  }

  /// Open the usb_device using the specified vendor and product id.
  /**
   * This function opens the usb device for the specified vendor and 
   * product id.
   *
   * @param vendor_id The vendor id for this usb device.
   * @param product_id The product id for this usb device.
   *
   * @param ec Set the indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID open(std::uint16_t vendor_id, 
      std::uint16_t product_id, boost::system::error_code& ec)
  {
    impl_.get_service().open(impl_.get_implementation(), vendor_id, 
        product_id, ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Assign an existing native usb device to the usb device.
  /*
   * This function opens the usb device to hold an existing native usb device.
   *
   * @param native_usb_device A native usb device.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void assign(const native_handle_type& native_usb_device)
  {
    boost::system::error_code ec;
    impl_.get_service().assign(impl_.get_implementation(),
        native_usb_device, ec);
    asio::detail::throw_error(ec, "assign");
  }

  /// Assign an existing native usb device to the usb device.
  /*
   * This function opens the usb device to hold an existing native usb device.
   *
   * @param native_usb_device A native usb device.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID assign(const native_handle_type& native_usb_device,
      boost::system::error_code& ec)
  {
    impl_.get_service().assign(impl_.get_implementation(),
        native_usb_device, ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Determine whether the usb device is open.
  bool is_open() const
  {
    return impl_.get_service().is_open(impl_.get_implementation());
  }

  /// Close the usb device.
  /**
   * This function is used to close the usb device. Any asynchronous read or
   * write operations will be cancelled immediately, and will complete with the
   * boost::system::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void close()
  {
    boost::system::error_code ec;
    impl_.get_service().close(impl_.get_implementation(), ec);
    asio::detail::throw_error(ec, "close");
  }

  /// Close the usb device.
  /**
   * This function is used to close the usb device. Any asynchronous read or
   * write operations will be cancelled immediately, and will complete with the
   * boost::system::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID close(boost::system::error_code& ec)
  {
    impl_.get_service().close(impl_.get_implementation(), ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Get the native usb device representation.
  /**
   * This function may be used to obtain the underlying representation of the
   * usb device. This is intended to allow access to native usb device
   * functionality that is not otherwise provided.
   */
  native_handle_type native_handle()
  {
    return impl_.get_service().native_handle(impl_.get_implementation());
  }

  /// Cancel all asynchronous operations associated with the usb device.
  /**
   * This function causes all outstanding asynchronous read or write operations
   * to finish immediately, and the handlers for cancelled operations will be
   * passed the boost::system::error::operation_aborted error.
   *
   * @throws boost::system::system_error Thrown on failure.
   */
  void cancel()
  {
    boost::system::error_code ec;
    impl_.get_service().cancel(impl_.get_implementation(), ec);
    asio::detail::throw_error(ec, "cancel");
  }

  /// Cancel all asynchronous operations associated with the usb device.
  /**
   * This function causes all outstanding asynchronous read or write operations
   * to finish immediately, and the handlers for cancelled operations will be
   * passed the boost::system::error::operation_aborted error.
   *
   * @param ec Set to indicate what error occurred, if any.
   */
  BOOST_ASIO_SYNC_OP_VOID cancel(boost::system::error_code& ec)
  {
    impl_.get_service().cancel(impl_.get_implementation(), ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Set an option on the usb device.
  /**
   * This function is used to set an option on the usb device.
   *
   * @param option The option value to be set on the usb device.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @sa SettableUsbDeviceOption @n 
   * usb_device_base::interface_number
   */
  template <typename SettableUsbDeviceOption>
  void set_option(const SettableUsbDeviceOption& option)
  {
    boost::system::error_code ec;
    impl_.get_service().set_option(impl_.get_implementation(), option, ec);
    asio::detail::throw_error(ec, "set_option");
  }

  /// Set an option on the usb device.
  /**
   * This function is used to set an option on the usb device.
   *
   * @param option The option value to be set on the usb device.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @sa SettableUsbDeviceOption @n 
   * usb_device_base::interface_number
   */
  template <typename SettableUsbDeviceOption>
  BOOST_ASIO_SYNC_OP_VOID set_option(const SettableUsbDeviceOption& option,
      boost::system::error_code& ec)
  {
    impl_.get_service().set_option(impl_.get_implementation(), option, ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Get an option from the usb device.
  /**
   * This function is used to get the current value of an option on the usb
   * device.
   *
   * @param option The option value to be obtained from the usb device.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @sa GettableUsbDeviceOption @n 
   * usb_device_base::interface_number
   */
  template <typename GettableUsbDeviceOption>
  void get_option(GettableUsbDeviceOption& option) const
  {
    boost::system::error_code ec;
    impl_.get_service().get_option(impl_.get_implementation(), option, ec);
    asio::detail::throw_error(ec, "get_option");
  }

  /// Get an option from the usb device.
  /**
   * This function is used to get the current value of an option on the usb
   * device.
   *
   * @param option The option value to be obtained from the usb device.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @sa GettableUsbDeviceOption @n 
   * usb_device_base::interface_number
   */
  template <typename GettableUsbDeviceOption>
  BOOST_ASIO_SYNC_OP_VOID get_option(GettableUsbDeviceOption& option,
      boost::system::error_code& ec) const
  {
    impl_.get_service().get_option(impl_.get_implementation(), option, ec);
    BOOST_ASIO_SYNC_OP_VOID_RETURN(ec);
  }

  /// Send some data to the usb device.
  /**
   * This function is used to send data to the usb device. The function call
   * will block until one or more bytes of the data has been written
   * successfully, or until an error occurs.
   *
   * @param buffers One or more data buffers to be written to the usb device.
   *
   * @returns The number of bytes written.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @par Example
   * To send a single data buffer use the @ref buffer function as follows:
   * @code
   * usb_device.send(asio::buffer(data, size));
   * @endcode
   * See the @ref buffer documentation for information on writing multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
   */
  template <typename ConstBufferSequence>
  std::size_t send(const ConstBufferSequence& buffers)
  {
    boost::system::error_code ec;
    std::size_t s = impl_.get_service().send(
        impl_.get_implementation(), buffers, ec);
    asio::detail::throw_error(ec, "send");
    return s;
  }

  /// Send some data to the usb device.
  /**
   * This function is used to send data to the usb device. The function call
   * will block until one or more bytes of the data has been written
   * successfully, or until an error occurs.
   *
   * @param buffers One or more data buffers to be written to the usb device.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns The number of bytes written. Returns 0 if an error occurred.
   */
  template <typename ConstBufferSequence>
  std::size_t send(const ConstBufferSequence& buffers,
      boost::system::error_code& ec)
  {
    return impl_.get_service().send(
        impl_.get_implementation(), buffers, ec);
  }
  
  /// Start an asynchronous send.
  /**
   * This function is used to asynchronously send data to the usb device.
   * The function call always returns immediately.
   *
   * @param buffers One or more data buffers to be written to the serial port.
   * Although the buffers object may be copied as necessary, ownership of the
   * underlying memory blocks is retained by the caller, which must guarantee
   * that they remain valid until the handler is called.
   *
   * @param handler The handler to be called when the write operation completes.
   * Copies will be made of the handler as required. The function signature of
   * the handler must be:
   * @code void handler(
   *   const boost::system::error_code& error, // Result of operation.
   *   std::size_t bytes_transferred           // Number of bytes written.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the handler will not be invoked from within this function. On
   * immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using asio::post().
   *
   * @par Example
   * To write a single data buffer use the @ref buffer function as follows:
   * @code
   * usb_device.async_send(
   *     asio::buffer(data, size), handler);
   * @endcode
   * See the @ref buffer documentation for information on writing multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
   */
  template <typename ConstBufferSequence, typename WriteHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler,
      void (boost::system::error_code, std::size_t))
  async_send(const ConstBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(WriteHandler) handler)
  {
    return asio::async_initiate<WriteHandler,
      void (boost::system::error_code, std::size_t)>(
        initiate_async_send(), handler, this, buffers);
  }

  /// Receive some data from the usb device.
  /**
   * This function is used to receive data from the usb device. The function
   * call will block until one or more bytes of data has been read successfully,
   * or until an error occurs.
   *
   * @param buffers One or more buffers into which the data will be read.
   *
   * @returns The number of bytes read.
   *
   * @throws boost::system::system_error Thrown on failure.
   *
   * @par Example
   * To read into a single data buffer use the @ref buffer function as follows:
   * @code
   * usb_device.receive(asio::buffer(data, size));
   * @endcode
   * See the @ref buffer documentation for information on reading into multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
   */
  template <typename MutableBufferSequence>
  std::size_t receive(const MutableBufferSequence& buffers)
  {
    boost::system::error_code ec;
    std::size_t s = impl_.get_service().receive(
        impl_.get_implementation(), buffers, ec);
    asio::detail::throw_error(ec, "receive");
    return s;
  }

  /// Receive some data from the usb device.
  /**
   * This function is used to receive data from the usb device. The function
   * call will block until one or more bytes of data has been read successfully,
   * or until an error occurs.
   *
   * @param buffers One or more buffers into which the data will be read.
   *
   * @param ec Set to indicate what error occurred, if any.
   *
   * @returns The number of bytes read. Returns 0 if an error occurred. 
   */
  template <typename MutableBufferSequence>
  std::size_t receive(const MutableBufferSequence& buffers,
      boost::system::error_code& ec)
  {
    return impl_.get_service().receive(
        impl_.get_implementation(), buffers, ec);
  }

  /// Start an asynchronous receive.
  /**
   * This function is used to asynchronously read data from the usb device.
   * The function call always returns immediately.
   *
   * @param buffers One or more buffers into which the data will be read.
   * Although the buffers object may be copied as necessary, ownership of the
   * underlying memory blocks is retained by the caller, which must guarantee
   * that they remain valid until the handler is called.
   *
   * @param handler The handler to be called when the read operation completes.
   * Copies will be made of the handler as required. The function signature of
   * the handler must be:
   * @code void handler(
   *   const boost::system::error_code& error, // Result of operation.
   *   std::size_t bytes_transferred           // Number of bytes read.
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the handler will not be invoked from within this function. On
   * immediate completion, invocation of the handler will be performed in a
   * manner equivalent to using asio::post().
   *
   * @par Example
   * To read into a single data buffer use the @ref buffer function as follows:
   * @code
   * usb_device.async_receive(
   *     asio::buffer(data, size), handler);
   * @endcode
   * See the @ref buffer documentation for information on reading into multiple
   * buffers in one go, and how to use it with arrays, boost::array or
   * std::vector.
   */
  template <typename MutableBufferSequence, typename ReadHandler>
  BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
      void (boost::system::error_code, std::size_t))
  async_receive(const MutableBufferSequence& buffers,
      BOOST_ASIO_MOVE_ARG(ReadHandler) handler)
  {
    return asio::async_initiate<ReadHandler,
      void (boost::system::error_code, std::size_t)>(
        initiate_async_receive(), handler, this, buffers);
  }

private:
  asio::detail::io_object_impl<detail::usb_device_service, Executor> impl_;

  // Disallow copying and assignment.
  usb_device(const usb_device&) BOOST_ASIO_DELETED;
  usb_device& operator=(const usb_device&) BOOST_ASIO_DELETED; 

  struct initiate_async_send
  {
    template <typename WriteHandler, typename ConstBufferSequence>
    void operator()(BOOST_ASIO_MOVE_ARG(WriteHandler) handler,
        usb_device* self, const ConstBufferSequence& buffers) const
    {
      BOOST_ASIO_WRITE_HANDLER_CHECK(WriteHandler, handler) type_check;

      asio::detail::non_const_lvalue<WriteHandler> handler2(handler);
      self->impl_.get_service().async_send(
          self->impl_.get_implementation(), buffers, handler2.value, 
          self->impl_.get_implementation_executor());
    }
  };

  struct initiate_async_receive
  {
    template <typename ReadHandler, typename MutableBufferSequence>
    void operator()(BOOST_ASIO_MOVE_ARG(ReadHandler) handler,
        usb_device* self, const MutableBufferSequence& buffers) const
    {
      BOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

      asio::detail::non_const_lvalue<ReadHandler> handler2(handler);
      self->impl_.get_service().async_receive(
          self->impl_.get_implementation(), buffers, handler2.value,
          self->impl_.get_implementation_executor());
    }
  };

};

} // namespace libusb
