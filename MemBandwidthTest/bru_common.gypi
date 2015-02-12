# copied from bru
{
    # Explicit default options for MSVC builds. Gyp itself does not
    # specify a lot of default project options for MSVC, to the point that
    # a simple type="executable" target won't even link with error
    #   LNK1561 entry point must be defined
    # because of missing /SUBSYSTEM:CONSOLE linker option.
    #
    # These defaults here were copied partially from
    #   https://github.com/joyent/node/blob/v0.10.26/common.gypi#L151
    "variables": {
        # That's for switching between 32bit and 64bit builds with msvc
        # on Windows.
        # E.g. switch to that via
        #   >set GYP_DEFINES=target_arch=x64
        # See also https://code.google.com/p/libyuv/wiki/GettingStarted
        # Alternatively we could allow switching between 32/64 bit
        # compilation via "configurations", but since libuv and nodejs
        # builds don't do that let's not bother doing this here either for now.
        "target_arch%": "ia32"
    },
    "target_defaults": {

        # To build config Debug with 'make' run:
        #   >make BUILDTYPE=Debug
        # To build config Debug it with msbuild run:
        #   >msbuild *.sln /p:Configuration=Debug
        "default_configuration": "Release",

        # List two configs: Debug & Release:
        "configurations": {
            "Debug": {
                "defines": [ "DEBUG", "_DEBUG" ],
                "cflags": [ "-g", "-O0" ],
                "conditions": [
                    ["target_arch=='x64'", {
                        "msvs_configuration_platform": "x64",
                    }],
                ],
                "msvs_settings": {
                    "VCCLCompilerTool": {
                        "RuntimeLibrary": 1, #static debug /MTd
                        "Optimization": 0, #/Od, no optimization
                        "MinimalRebuild": "false",
                        "OmitFramePointers": "false",
                        "BasicRuntimeChecks": 3, #/RTC1
                    },
                    "VCLinkerTool": {
                        "LinkIncremental": 1, # 2 would enable, but conflicts with LTCG
                    },
                },
                "xcode_settings": {
                    "GCC_OPTIMIZATION_LEVEL": "0", #stop gyp from defaulting to - Os
                },
            },
            "Release": {
                "defines": [
                    # should we add this here or not? E.g. rcf won't compile
                    # unless NDEBUG is set for release builds. This is more of
                    # a Windows/msvs thing than a Linux thing. Let's have
                    # downstream projects set this explicitly if they require
                    # it? Or just set it here? Let's set here for now:
                    "NDEBUG"
                ],
                "cflags": [
                    "-O2",
                    #? "-fno-strict-aliasing",
                    "-ffunction-sections", "-fdata-sections" ],
                "ldflags": [ "-Wl,--gc-sections" ],
                "conditions": [
                    ["target_arch=='x64'", {
                        "msvs_configuration_platform": "x64",
                    }]
                ],
                "msvs_settings": {
                    "VCCLCompilerTool": {
                        "RuntimeLibrary": 0, # static release
                        "Optimization": 3, # /Ox, full optimization
                        "FavorSizeOrSpeed": 1, # /Ot, favour speed over size
                        "InlineFunctionExpansion": 2, # /Ob2, inline anything eligible
                        "WholeProgramOptimization": "true", # /GL, whole program optimization, needed for LTCG
                        "OmitFramePointers": "true",
                        "EnableFunctionLevelLinking": "true",
                        "EnableIntrinsicFunctions": "true"
                    }
                },
                "VCLibrarianTool": {
                    "AdditionalOptions": [
                        "/LTCG" # link time code generation
                    ]
                },
                "VCLinkerTool": {
                    "LinkTimeCodeGeneration": 1, # link-time code generation
                    "OptimizeReferences": 2, # /OPT:REF
                    "EnableCOMDATFolding": 2, # /OPT:ICF
                    "LinkIncremental": 1 # disable incremental linking
                }
            }
        },

        # now settings that are identical across configurations Debug/Release
        "msvs_settings": {
            "VCCLCompilerTool": {
                "StringPooling": "true", # pool string literals
                "DebugInformationFormat": 3, # Generate a PDB
                "WarningLevel": 3,
                "BufferSecurityCheck": "true",
                "ExceptionHandling": 1, # /EHsc
                "SuppressStartupBanner": "true",
                "WarnAsError": "false"
                #"RuntimeTypeInfo": "false",
            },
            "VCLinkerTool": {
                "conditions": [
                      ["target_arch=='x64'", {
                            "TargetMachine" : 17 # /MACHINE:X64
                      }]
                ],
                "GenerateDebugInformation": "true",
                #"RandomizedBaseAddress": 2, # enable ASLR
                #"DataExecutionPrevention": 2, # enable DEP
                #"AllowIsolation": "true",
                "SuppressStartupBanner": "true",
                "LinkTimeCodeGeneration": 1, # link-time code generation
                "OptimizeReferences": 2, # /OPT:REF
                "EnableCOMDATFolding": 2, # /OPT:ICF
                "LinkIncremental": 1, # disable incremental linking
                #"ImageHasSafeExceptionHandlers": "false", # /SAFESEH:NO
                "target_conditions": [
                    ["_type=='executable'", {
                        "SubSystem": 1 # console executable
                    }]
                ]
            }
        }
    }
}
