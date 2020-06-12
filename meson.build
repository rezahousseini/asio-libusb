project('asio_libusb', ['c', 'cpp'], 
  default_options: ['warning_level=3', 'c_std=gnu11', 'cpp_std=gnu++17'])

inc = include_directories('include')

libusb_dep = dependency('libusb-1.0')

asio_libusb_dep = declare_dependency(
  dependencies : libusb_dep,
  include_directories : inc,
)

if get_option('ASIO_LIBUSB_BUILD_TESTS')
  subdir('test')
endif

if get_option('ASIO_LIBUSB_BUILD_EXAMPLES')
  subdir('examples')
endif
