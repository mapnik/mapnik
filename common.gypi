{
  'target_defaults': {
    'default_configuration': 'Release',
    'msbuild_toolset':'v140',
    'msvs_configuration_platform': 'Win32',
    'msvs_disabled_warnings': [ 4068,4244,4005,4506,4345,4804,4805,4661 ],
    'xcode_settings': {
      'CLANG_CXX_LIBRARY': 'libc++',
      'CLANG_CXX_LANGUAGE_STANDARD':'c++11',
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
            'ObjectFile': '$(IntDir)/%(RelativeDir)/', # support similiarly named files in different directories
            'ExceptionHandling': 1, # /EHsc
            'RuntimeTypeInfo': 'true', # /GR
            'RuntimeLibrary': '2' # 2:/MD
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
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': '3', # /MDd
            'Optimization': 0, # /Od, no optimization
            'MinimalRebuild': 'false',
            'OmitFramePointers': 'false',
            'BasicRuntimeChecks': 3 # /RTC1
          },
          'VCLinkerTool': {
            'AdditionalOptions': [
              #'/NODEFAULTLIB:msvcrt.lib'
            ]
          }
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
        },
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': '2', #0:/MT, 2:/MD, 
            'Optimization': 3, # /Ox, full optimization
            'FavorSizeOrSpeed': 1, # /Ot, favour speed over size
            'InlineFunctionExpansion': 2, # /Ob2, inline anything eligible
            #'WholeProgramOptimization': 'true', # /GL, whole program optimization, needed for LTCG
            'OmitFramePointers': 'true',
            #'EnableFunctionLevelLinking': 'true',
            'EnableIntrinsicFunctions': 'true',
            'AdditionalOptions': [
              '/MP', # compile across multiple CPUs
            ],
            'DebugInformationFormat': '0'
          },
          'VCLibrarianTool': {
            'AdditionalOptions': [
              '/LTCG' # link time code generation
            ],
          },
          'VCLinkerTool': {
            #'LinkTimeCodeGeneration': 1, # link-time code generation
            #'OptimizeReferences': 2, # /OPT:REF
            #'EnableCOMDATFolding': 2, # /OPT:ICF
            'LinkIncremental': 2, # force incremental linking
            'GenerateDebugInformation': 'false',
            'AdditionalOptions': [
                #'/NODEFAULTLIB:libcmt.lib'
            ],
          }
        }
      }
    }
  }
}
