{
  'target_defaults': {
    'default_configuration': 'Release',
    'xcode_settings': {
      'CLANG_CXX_LIBRARY': 'libstdc++',
      'CLANG_CXX_LANGUAGE_STANDARD':'c++03',
      'GCC_VERSION': 'com.apple.compilers.llvm.clang.1_0',
      'MACOSX_DEPLOYMENT_TARGET':'10.9',
      'WARNING_CFLAGS': [
        '-Wall',
        '-Wextra',
        '-pedantic',
        '-Wno-parentheses',
        '-Wno-char-subscripts',
        '-Wno-unused-parameter',
        '-Wno-c++11-narrowing',
        '-Wno-c++11-long-long',
        '-Wno-unsequenced',
        '-Wno-sign-compare',
        '-Wno-unused-function',
        '-Wno-redeclared-class-member',
        '-Wno-c99-extensions',
        '-Wno-c++11-extra-semi',
        '-Wno-variadic-macros',
        '-Wno-c++11-extensions',
        '-Wno-unused-const-variable'
      ]
    },
    'msvs_settings': {
        'VCCLCompilerTool': {
            'ExceptionHandling': 1, # /EHsc
            'RuntimeTypeInfo': 'true', # /GR
            'RuntimeLibrary': '2' # /MD
        }
    },
    'defines': [ 'BOOST_SPIRIT_USE_PHOENIX_V3=1' ],
    'cflags_cc': ['-std=c++03'],
    'conditions': [
      ['OS=="win"', {
        'defines': ['_WINDOWS']
      }]
    ],
    'configurations': {
      'Debug': {
        'defines!': [
            'NDEBUG'
        ],
        'cflags_cc!': [
            '-O3',
            '-O2',
            '-Os',
            '-DNDEBUG'
        ],
        'cflags': [ '-g', '-O0' ],
        'defines': [ 'DEBUG' ],
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0',
          'GCC_GENERATE_DEBUGGING_SYMBOLS': 'YES',
          'DEAD_CODE_STRIPPING': 'NO',
          'GCC_INLINES_ARE_PRIVATE_EXTERN': 'NO'
        }
      },
      'Release': {
        'cflags': [ '-O3' ],
        'defines': [ 'NDEBUG' ],
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '3',
          'GCC_GENERATE_DEBUGGING_SYMBOLS': 'NO',
          'DEAD_CODE_STRIPPING': 'YES',
          'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES'
        }
      }
    }
  }
}
