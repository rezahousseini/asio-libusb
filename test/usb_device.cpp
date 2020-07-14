#include <boost/ut.hpp>
#include <boost/asio.hpp>
#include "libusb/usb_device.hpp"
#include "libusb/usb_device_acceptor.hpp"

int main()
{
  using namespace boost::ut;
  using namespace libusb;
  namespace asio = boost::asio;

  "usb device"_test = []
  {
    asio::io_context io_context;
    usb_device_acceptor acceptor(io_context);
    auto device = std::make_shared<usb_device<>>(io_context);

    acceptor.async_accept(device->lowest_layer(), 0xdead, 0xbeef, 
        [](const boost::system::error_code& ec)
        {
          expect(!ec) << ec;
        });

    io_context.run();

    device->open();
    
    expect(device->is_open());

    should("get endpoint address") = [&device]
    {
      usb_device_base::endpoint_address option;
      device->get_option(option);
      expect(0_uc == option.value());
    };
    
    should("set endpoint address") = [&device]
    {
      usb_device_base::endpoint_address option(0x01);
      device->set_option(option);
    };

    should("async send") = [&io_context, &device]
    {
      usb_device_base::endpoint_address option;
      device->get_option(option);
      expect(0x01 == option.value());

      io_context.restart();

      std::vector<std::byte> command;
      command.push_back(static_cast<std::byte>(0));
      
      expect(1_ul == command.size());

      device->async_send(asio::buffer(command), 
        [&](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
          expect(!ec) << ec;
          expect(1_ul == bytes_transferred); 
        }); 

      io_context.run();
    };

    should("async receive") = [&io_context, &device]
    {
      io_context.restart(); 

      std::vector<std::byte> data(1024);

      device->async_receive(asio::buffer(data), 
        [&](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
          std::cout << "Actual length: " << bytes_transferred << std::endl;
          expect(!ec) << ec;
          expect(1024_ul == bytes_transferred);
        });

      io_context.run();
    };
  }; 
}
