{
  'targets': [
    {
      'target_name': 'node-libvirt',
      'sources': [ 
        'src/node-libvirt.cpp',
        'src/hypervisor.cpp',
       ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")","/usr/include/libvirt"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'libraries': ["/usr/lib/x86_64-linux-gnu/libvirt.so.0"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      "link_settings": {
                      "libraries": [
                          "<!@(pkg-config --libs libvirt)"
                      ]
             }
    }
  ]
}