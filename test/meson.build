boost_ut_dep = dependency('boost_ut', fallback : ['boost_ut', 'boost_ut_dep'])

progs = [
  'usb_device',
  'acceptor',
]

foreach p : progs
  exe = executable(p.underscorify(),
    p + '.cpp',
    dependencies : [
      boost_ut_dep, 
      asio_libusb_dep,
    ],
    cpp_args : '-Wno-pedantic'
  )
  test(p.underscorify(), exe)
endforeach
