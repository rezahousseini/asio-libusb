#pragma once

#include <boost/asio.hpp>

namespace libusb {

class usb_device_base
{
public:
  /// Usb device option to permit changing the interface number.
  /**
   * Implements changing the interfcae number for a given usb device.
   */
  class interface_number
  {
  public:
    explicit interface_number(int t = 0)
      : value_(t)
    {
    }

    int value() const
    {
      return value_;
    }
 
  private:
    int value_;
  };

  /// Usb device option to permit changing the endpoint address.
  /**
   * Implements changing the endpoint address for a given usb device.
   */
  class endpoint_address
  {
  public:
    explicit endpoint_address(std::uint8_t t = 0)
      : value_(t)
    {
    }

    int value() const
    {
      return value_;
    }
 
  private:
    int value_;
  };

protected:
  /// Protected destructor to prevent deletion through this type.
  ~usb_device_base()
  {
  }
};

} // namespace libusb
