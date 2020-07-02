#pragma once

#include <libusb.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace boost
{
  namespace system
  {
    // Tell the C++ 11 STL metaprogramming that enum ConversionErrc
    // is registered with the standard error code system
    template <> struct is_error_code_enum<libusb_error> : std::true_type
    {
    };
  }  // namespace system
}  // namespace boost

namespace detail
{
  // Define a custom error code category derived from boost::system::error_category
  class libusb_error_category : public boost::system::error_category
  {
  public:
    // Return a short descriptive name for the category
    virtual const char *name() const noexcept override final { return "libusb_error"; }
    // Return what each enum means in text
    virtual std::string message(int c) const override final
    {
      return libusb_error_name(static_cast<libusb_error>(c)); 
    }
    // OPTIONAL: Allow generic error conditions to be compared to me
    virtual boost::system::error_condition default_error_condition(int c) const noexcept override final
    {
      switch(static_cast<libusb_error>(c))
      {
      case libusb_error::LIBUSB_SUCCESS:
        return make_error_condition(boost::system::errc::success);
      case libusb_error::LIBUSB_ERROR_IO:
        return make_error_condition(boost::system::errc::io_error);
      case libusb_error::LIBUSB_ERROR_INVALID_PARAM:
        return make_error_condition(boost::system::errc::invalid_argument);
      case libusb_error::LIBUSB_ERROR_ACCESS:
        return make_error_condition(boost::system::errc::result_out_of_range);
      case libusb_error::LIBUSB_ERROR_NO_DEVICE:
        return make_error_condition(boost::system::errc::no_such_device);
      case libusb_error::LIBUSB_ERROR_NOT_FOUND:
        return make_error_condition(boost::system::errc::host_unreachable);
      case libusb_error::LIBUSB_ERROR_BUSY:
        return make_error_condition(boost::system::errc::device_or_resource_busy);
      case libusb_error::LIBUSB_ERROR_TIMEOUT:
        return make_error_condition(boost::system::errc::timed_out);
      case libusb_error::LIBUSB_ERROR_OVERFLOW:
        return make_error_condition(boost::system::errc::no_buffer_space);
      case libusb_error::LIBUSB_ERROR_PIPE:
        return make_error_condition(boost::system::errc::broken_pipe);
      case libusb_error::LIBUSB_ERROR_INTERRUPTED:
        return make_error_condition(boost::system::errc::interrupted);
      case libusb_error::LIBUSB_ERROR_NO_MEM:
        return make_error_condition(boost::system::errc::no_space_on_device);
      case libusb_error::LIBUSB_ERROR_NOT_SUPPORTED:
        return make_error_condition(boost::system::errc::not_supported);
      default:
        // I have no mapping for this code
        return boost::system::error_condition(c, *this);
      }
    }
  };
}  // namespace detail

// Define the linkage for this function to be used by external code.
// This would be the usual __declspec(dllexport) or __declspec(dllimport)
// if we were in a Windows DLL etc. But for this example use a global
// instance but with inline linkage so multiple definitions do not collide.
#define THIS_MODULE_API_DECL extern inline

// Declare a global function returning a static instance of the custom category
THIS_MODULE_API_DECL const detail::libusb_error_category &libusb_error_category()
{
  static detail::libusb_error_category c;
  return c;
}


// Overload the global make_error_code() free function with our
// custom enum. It will be found via ADL by the compiler if needed.
inline boost::system::error_code make_error_code(libusb_error e)
{
  return {static_cast<int>(e), libusb_error_category()};
}
