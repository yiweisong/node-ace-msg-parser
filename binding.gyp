{
  'targets': [
    {
      'target_name': 'ace-msg-parser',
      'sources': [ 
        'src/addon.cc',
        'src/utils.cc',
        'src/message_extractor.cc',
        'src/analyzers/nmea_analyzer.cc',
        'src/analyzers/aceinna_binary_analyzer.cc',
      ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      }
    }
  ],
  'variables': {
    'openssl_fips': '',
    'openssl_is_fips': 'false',
    'openssl_quic': 'true',
  }
}