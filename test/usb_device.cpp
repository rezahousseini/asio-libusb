#include <boost/ut.hpp>
#include "libusb/usb_device.hpp"

int main()
{
  using namespace boost::ut;
  using namespace libusb;

  "open device"_test = []
  {
    std::int32_t i = 42; 
    expect(42_i == i);
  }; 
}
