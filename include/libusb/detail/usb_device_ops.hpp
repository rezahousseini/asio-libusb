#pragma once
#include <libusb.h>
#include <boost/asio.hpp>

namespace asio = boost::asio;

namespace libusb {
namespace detail {
namespace usb_device_ops {

bool process_transfer(struct libusb_context* ctx, 
    struct libusb_transfer* transfer, int* transfer_complete, 
    boost::system::error_code& ec)
{
  int err = libusb_submit_transfer(transfer);
  if (err != LIBUSB_SUCCESS)
  {
    ec = libusb_error(err);
    return false;
  }
  
  err = libusb_handle_events_completed(ctx, transfer_complete);
  ec = libusb_error(err);
  return err == LIBUSB_SUCCESS and transfer_complete;
}

bool find_device(struct libusb_context* ctx, 
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

} // namespace usb_device_ops
} // namespace detail
} // namespace libusb
