#include <memory>
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
    auto device = std::make_shared<usb_device<>>(io_context);

    acceptor.async_accept(device->lowest_layer(), 0xdead, 0xbeef, 
        [device](const boost::system::error_code& ec)
        {
          expect(!ec) << ec;
          device->open();
          expect(device->is_open());
        });

    io_context.run(); 
  }; 
}
