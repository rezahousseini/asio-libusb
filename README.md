# Asio wrapper for libusb

## Structure

 * `libusb/usb_device.hpp` IO object for a usb device
 * `libusb/usb_device_acceptor.hpp` IO object to accept new usb devices (hotplug)
 * `libusb/detail/usb_device_service.hpp` Low level object for calls to libusb library
 * `libusb/detail/async_transfer_op.hpp` Asynchronous USB transfer operator
 * `libusb/detail/async_accept_op.hpp` Asynchronous accept operator

## Building

 * Initialize: `meson build`
 * Build: `ninja -C build`
