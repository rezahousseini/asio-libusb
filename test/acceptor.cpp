#include <boost/ut.hpp>
#include "libusb/usb_device_acceptor.hpp"

int main()
{
  using namespace boost::ut;
  using namespace libusb;
  namespace asio = boost::asio;

  "asynchronous accept"_test = []
  {
    asio::io_context io_context;
    usb_device_acceptor acceptor(io_context);
    usb_device device(io_context);
    boost::system::error_code my_ec;

    acceptor.async_accept(device, 0xdead, 0xbeef, 
        [&my_ec](const boost::system::error_code& ec)
        {
          my_ec = ec;
        });
    io_context.run();
    
    expect(!my_ec) << my_ec;
    expect(device.is_open());
  }; 
}
