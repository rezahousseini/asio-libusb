#pragma once

#include <atomic>
#include <boost/asio.hpp>

#include "libusb/detail/usb_device_service.hpp"

namespace libusb {
namespace detail { 

void usb_device_service::assign(usb_device_service::implementation_type& impl, 
    native_handle_type native_usb_device, boost::system::error_code& ec)
{
  if (is_open(impl))
  {
    ec = asio::error::already_open;
    return;
  }

  impl.device_ = native_usb_device;
}

bool usb_device_service::is_open(const implementation_type& impl) const
{ 
  if (impl.dev_handle_)
  {
    int err = libusb_claim_interface(impl.dev_handle_, 
        impl.interface_number_.value());
    return err == LIBUSB_SUCCESS;
  }
  
  return false;
}

void usb_device_service::open(implementation_type& impl, 
    boost::system::error_code& ec)
{
  if (is_open(impl))
  {
    ec = boost::asio::error::already_open;
    return;
  } 

  int err = libusb_open(impl.device_, &impl.dev_handle_);
  if (err != LIBUSB_SUCCESS)
  {
    ec = libusb_error(err);
    return;
  }

  err = libusb_claim_interface(impl.dev_handle_,  
      impl.interface_number_.value());
  ec = libusb_error(err);
}

void usb_device_service::close(implementation_type& impl, 
    boost::system::error_code& ec)
{
  if (is_open(impl))
  {
    int err = libusb_release_interface(impl.dev_handle_, 
        impl.interface_number_.value());
    ec = libusb_error(err);
    libusb_close(impl.dev_handle_);
  }
}

usb_device_service::native_handle_type usb_device_service::native_handle(
    implementation_type& impl)
{
  return impl.device_;
}

void usb_device_service::do_set_option(implementation_type& impl, 
      const usb_device_base::interface_number& option, 
      boost::system::error_code& ec)
{
  int rc = libusb_release_interface(impl.dev_handle_, 
      impl.interface_number_.value());
  if (rc != LIBUSB_SUCCESS and rc != LIBUSB_ERROR_NOT_FOUND)
  {
    ec = libusb_error(rc);
    return;
  }

  rc = libusb_claim_interface(impl.dev_handle_, option.value());
  if (rc == LIBUSB_SUCCESS)
    impl.interface_number_ = option;
  ec = libusb_error(rc);
}

void usb_device_service::do_set_option(implementation_type& impl, 
      const usb_device_base::endpoint_address& option, 
      boost::system::error_code& /*ec*/)
{
  impl.endpoint_address_ = option;
}

void usb_device_service::do_get_option(const implementation_type& impl, 
      usb_device_base::endpoint_address& option, 
      boost::system::error_code& /*ec*/) const
{
  option = impl.endpoint_address_;
}

void usb_device_service::do_get_option(const implementation_type& impl, 
      usb_device_base::interface_number& option, 
      boost::system::error_code& /*ec*/) const
{
  option = impl.interface_number_;
}

template <typename ConstBufferSequence>
std::size_t usb_device_service::send(implementation_type& impl, 
    const ConstBufferSequence& buffers, boost::system::error_code& ec)
{
  asio::detail::buffer_sequence_adapter<asio::const_buffer, 
  ConstBufferSequence> bufs(buffers);

  std::size_t bytes_transferred;

  int rc = libusb_interrupt_transfer(
      impl.dev_handle_,
	    impl.endpoint_address_.value(),
	    bufs.buffers(),
	    bufs.count(),
	    &bytes_transferred,
      0);

  ec = libusb_error(rc);

  return bytes_transferred;
}

template <typename MutableBufferSequence>
size_t usb_device_service::receive(implementation_type& impl,
    const MutableBufferSequence& buffers, boost::system::error_code& ec)
{
  asio::detail::buffer_sequence_adapter<asio::mutable_buffer, 
  MutableBufferSequence> bufs(buffers);

  std::size_t bytes_transferred;

  int rc = libusb_interrupt_transfer(
      impl.dev_handle_,
	    impl.endpoint_address_.value() + 128,
	    bufs.buffers(),
	    bufs.count(),
	    &bytes_transferred,
      0);

  ec = libusb_error(rc);

  return bytes_transferred;
}

} // namespace detail
} // namespace libusb
