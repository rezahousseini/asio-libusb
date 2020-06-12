// Copyright (c) Benjamin Kietzman (github.com/bkietz)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <libusb.h>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace libusb {

class error : public boost::system::error_category
{
public:
  error()
  {
    error_ = LIBUSB_SUCCESS;
  }

  error(libusb_error src)
  {
    error_ = src;
  }

  const char* name() const BOOST_NOEXCEPT
  {
    return libusb_error_name(error_);
  }

  /* boost::system::error_condition default_error_condition(int ev) const BOOST_NOEXCEPT {} */

  /* bool equivalent(int code, const boost::system::error_condition& condition) const BOOST_NOEXCEPT {} */

  /* bool equivalent(const boost::system::error_code& code, int condition) const BOOST_NOEXCEPT {} */

  std::string message(int /*ev*/) const
  {
    return libusb_strerror(error_);
  }

  /* char const* message(int ev, char* buffer, std::size_t len) const BOOST_NOEXCEPT {} */

  /* bool failed(int ev) const BOOST_NOEXCEPT {} */

  bool is_set() const
  {
    return error_ != LIBUSB_SUCCESS;
  } 

  operator libusb_error ()
  {
    return error_;
  }

  boost::system::error_code error_code() const
  {
    return boost::system::error_code(is_set(), *this);
  }

  boost::system::system_error system_error() const
  {
    return boost::system::system_error(error_code());
  } 

private:  
  libusb_error error_;
};

} // namespace libusb
