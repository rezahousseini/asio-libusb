#pragma once

#include <atomic>
#include <boost/asio.hpp>

#include "libusb/detail/usb_device_service.hpp"

namespace libusb {
namespace detail { 

void usb_device_service::assign(usb_device_service::implementation_type& impl, 
    native_handle_type& native_usb_device, boost::system::error_code& /*ec*/)
{
  impl.dev_handle_ = native_usb_device;
}

bool usb_device_service::is_open(const implementation_type& impl) const
{
  return impl.dev_handle_ != NULL;
}

template <typename IoExecutor>
void usb_device_service::open(implementation_type& impl, 
    std::uint16_t vendor_id, std::uint16_t product_id, 
    boost::system::error_code& ec, const IoExecutor& io_ex)
{
  libusb_device** devs;
  if (is_open(impl))
  {
    ec = boost::asio::error::already_open;
    return;
  }

  int cnt = libusb_get_device_list(impl.ctx_, &devs);
  if (cnt < 0)
  {
    impl.convert_error_free_device_list(cnt, ec, devs);
	  return;
  }

  int index;
  for (index = 0; index < cnt; index++)
  {
    struct libusb_device_descriptor desc;

    auto ret = libusb_get_device_descriptor(devs[index], &desc);
    if (ret < 0)
    {
      impl.convert_error_free_device_list(ret, ec, devs);
	    return;
    }
    if (desc.idVendor == vendor_id && desc.idProduct == product_id)
    {
      break;  
    }
    if (index == cnt - 1)
    {
      impl.convert_error_free_device_list(LIBUSB_ERROR_NOT_FOUND, ec, devs);
	    return;
    }
  }

  auto err = libusb_open(devs[index], &impl.dev_handle_);
  if (err != LIBUSB_SUCCESS)
  {
    impl.convert_error_free_device_list(err, ec, devs); 
    return;
  }

  // claim interface
  do_set_option(impl, impl.interface_number_, ec);

  // start event handling
  impl.handle_events(io_ex);
}

void usb_device_service::close(implementation_type& impl, 
    boost::system::error_code& /*ec*/)
{
  impl.do_handle_events_ = false;
  libusb_close(impl.dev_handle_);
}

usb_device_service::native_handle_type usb_device_service::native_handle(
    implementation_type& impl)
{
  return impl.dev_handle_;
}

void usb_device_service::cancel(implementation_type& impl, 
      boost::system::error_code& /*ec*/)
{
  impl.do_handle_events_ = false;
}

void usb_device_service::do_set_option(implementation_type& impl, 
      const usb_device_base::interface_number& option, 
      boost::system::error_code& ec)
{
  int rc = libusb_release_interface(impl.dev_handle_, 
      impl.interface_number_.value());
  if (rc != LIBUSB_SUCCESS and rc != LIBUSB_ERROR_NOT_FOUND)
  {
    ec = error(libusb_error(rc)).error_code();
    return;
  }

  rc = libusb_claim_interface(impl.dev_handle_, option.value());
  if (rc == LIBUSB_SUCCESS)
    impl.interface_number_ = option;
  ec = error(libusb_error(rc)).error_code();
}

void usb_device_service::do_set_option(implementation_type& impl, 
      const usb_device_base::endpoint_address& option, 
      boost::system::error_code& /*ec*/)
{
  impl.endpoint_address_ = option;
}

void usb_device_service::do_get_option(implementation_type& impl, 
      usb_device_base::endpoint_address& option, 
      boost::system::error_code& /*ec*/)
{
  option = impl.endpoint_address_;
}

void usb_device_service::do_get_option(implementation_type& impl, 
      usb_device_base::interface_number& option, 
      boost::system::error_code& /*ec*/)
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

  ec = error(libusb_error(rc)).error_code();

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
	    impl.endpoint_address_.value(),
	    bufs.buffers(),
	    bufs.count(),
	    &bytes_transferred,
      0);

  ec = error(libusb_error(rc)).error_code();

  return bytes_transferred;
}

} // namespace detail
} // namespace libusb
