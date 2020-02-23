# META - cross platform build system generator
- [META - cross platform build system generator](#meta---cross-platform-build-system-generator)
  - [What Meta is](#what-meta-is)
  - [What Meta NOT is](#what-meta-not-is)
- [Motivation and Influence](#motivation-and-influence)
- [Configuration](#configuration)
  - [Image](#image)
  - [Package](#package)
  - [Machines](#machines)
  - [Toolchain](#toolchain)
  - [Project and User Options](#project-and-user-options)
- [Supported OS](#supported-os)
- [Limitations](#limitations)
- [How to build serenity](#how-to-build-serenity)

## What Meta is

* Meta is a build system file generator for C, C++, ASM
  * Thus it is possible to extend meta to generate the build files for the desired build system.
* Meta is based on the concept of images that contain packages
  * This is flexible enough to build and run images created with the exact packages intended to be within that image
* Descriptive
  * This is a crucial part of the concept: The build files contain the absolute minimum that is needed to build the software. There is no possibility to implement corner-cases in the build files. This moves to the generator. Corner cases and special things must be implemented in the respective build system generator.
* Single source
  * For some build systems (e.g. yocto/openembedded) you need descriptions for the source packages and the source packages itself. With generating both, the descriptive meta files act as single source for multi staged build systems.

## What Meta NOT is

* Another build system
  * Build systems are complex and you can do a lot wrong if you don't consider all cases. Meta's aim is not to provide another build system, but to abstract basically all build systems that you can generate the build files for them


# Motivation and Influence
The concept of meta is heavily influenced by [this](https://mropert.github.io/2018/04/27/simplifying_build-part3) article fomr Mathieu Ropert and [this](https://www.youtube.com/watch?v=mWOmkwv8N_U) talk from Peter Bindels at the cppcon 2018.

The motivation is to provide a build system abstraction that lets users speficy the things that are relevant, instead of scripting ton's of build system related files. Especially with bigger projects, maintainance of the build system becomse a crucial and time intensive part.

Switching build systems is hard, if not the hardest task for big projects. Hundrets of workarounds, corner cases, and so on are scripted into the used build system. To switch, all corner cases have to be supported by or newly implemented in the target build system.
With meta, it's pretty easy to switch to a new build system, as the generator for that build system exists. For special demands, the generator could be adapted for the project or meta could be extended for new use-cases.
It's also easy possible to switch from meta with it's descriptive meta files completely off to a different system. By writing a generator that transforms the JSON to the output format you need.

Developer focus: Developers shall focus on the software, not on the build system. Nowadays build systems are that complex, that more and more time goes into maintaining and understanding the build system instead of writing productively code.

# Configuration
All configuration is done in JSON files. (TODO: JSON schema to be provided).

## Image
```JSON
{
    "image": {
        "default-image": {
            "install": ["packageA", "packageB", ...] | "*",
            "install_prefix": "/usr",
            "install_dirs": {
                "BinDir": "bin",
                "LibDir": "lib",
                "IncludeDir": "include",
                "ManDir": "share/man"
            }
        }
    }
}
```

To decribe the content of the image briefly:
* The name of the image is `default-image`.
* It contains a list of packages that shall be installed `packageA` and `packageB` or you can use the `*` operator to indicate all available packages shall be installed.
* Packages shall be install to `/usr` and below, the directory structure is given by `install_dirs`.

## Package

There are different types of packages:

1. `executable`
2. `library`
3. `collection`
4. `deployment`

For each package type, the structure slightly differs. `executable` and `library` have the same format, shown in the next code block:

```JSON
{
    "package": {
        "LibXY": {
            "type": "library",
            "source": [
                "${root}/Libraries/LibXY/*.cpp",
                "${root}/Libraries/LibXY/a_file.c",
                "${root}/Libraries/LibXY/another_file.S"
            ],
            "include": [
                "${root}/Libraries/LibXY"
            ],
            "dependency": [
                "LibA",
                "LibB",
                "LibC"
            ],
            "deploy": [
                {
                    "type": "target",
                    "name": "xyz",
                    "dest": "${LibDir}"
                },
                {
                    "type": "directory",
                    "source": "${root}/Libraries/LibXY/",
                    "pattern": "*.h",
                    "dest": "${IncludeDir}/LibXYZ"
                }
            ]
        }
    }
}
```
To decribe the content of the package briefly:
* The name of the package is `LibXY`.
* It contains a list of sources and includes that are used to build it. Glob's are supported but are not mandatory. Variables can be used for filenames nearly everywhere (TODO: has to be improved a bit :-) ). In this case, all files are referenced from `${root}` which is the project root specified by the project settings below. Without a variable, the filenames are relative to the path where the JSON file is stored.
* `dependency` is a list of packages, this package relies on.
* `deploy` is information that brings specific files of a target into the filesystem of the `image` to be built. There are different `type`'s of deployment: `target`, `file`, `directory`, `symlink` (TBD), `object` and `program`. Basically you define a `source` a destination `dest` and some specific parameters for each type.


Two things have been left out of the example, described later:
* `host_tools` with this object, you have the possibility to further specify requirement on the host tools, like flags for the c++ compiler.
* `run_generators` with this object, you have the possibility to specify requirements on generators that provide files for your compilation.

## Machines
TODO: Introduce concept of machines

## Toolchain
The toolchain contains settings for the compilers and tools that are used, on the build machine and the host machine.

```JSON
{
    "toolchain": {
        "default": {
            "configuration": {
                "Debug": {
                    "cxx": {
                        "flags": "-Og -DDEBUG -DSANITIZE_PTRS"
                    },
                    "cc": {
                        "flags": "-Og -DDEBUG -DSANITIZE_PTRS"
                    }
                },
                "Release": {
                    "cxx": {
                        "flags": "-Os -DRELEASE"
                    },
                    "cc": {
                        "flags": "-Os -DRELEASE"
                    }
                },
                "MinSizeRel": {},
                "DebWithRelInfo": {},
                ...
            },
            "build_tools": {
                "cxx": {
                    "executable": "g++",
                    "flags": [
                        "-MMD -MP -std=c++17"
                    ]
                },
                "cc": {
                    "executable": "gcc",
                    "flags": [
                        "-MMD -MP"
                    ]
                },
                "as": {
                    "executable": "as",
                    "flags": ""
                },
                "link": {
                    "executable": "ld",
                    "flags": ""
                },
                "ranlib": {
                    "executable": "ranlib",
                    "flags": ""
                },
                "ar": {
                    "executable": "ar",
                    "flags": ""
                },
                "download": {
                    "executable": "wget",
                    "flags": "-O-"
                },
                "patch": {
                    "executable": "patch",
                    "flags": ""
                },
                "build": {
                    "executable": "make",
                    "flags": ""
                },
                "install": {
                    "executable": "make",
                    "flags": "install"
                },
                "build-generator": {
                    "executable": "cmake",
                    "flags": ""
                }
            },
            "host_tools": {
                "cxx": {
                    "executable": "xy-g++",
                    "flags": [
                        "-MMD -MP -std=c++17"
                    ]
                },
                "cc": {
                    "executable": "xy-gcc",
                    "flags": [
                        "-MMD -MP"
                    ]
                },
                "build_image": {
                    "executable": "build-image-qemu.sh",
                    "flags": "${root}",
                    "add_as_target": true,
                    "run_as_su": true
                },
                "run": {
                    "executable": "run.sh",
                    "flags": "${target_sysroot}/boot/kernel",
                    "add_as_target": true
                }
            },

            "build_machine_build_targets": [
                "libstdc++-v3",
                "qemu"
            ]
        }
    }
}
```
To decribe the content of the toolchain briefly:
* The name of the toolchain is `default`.
* It contains a `configuration` that provides set's of different flags for specific build configurations.
* It contains a list of `build_tools` that specify the executable names and the flags for the tools used in the build toolchain.
* It contains a list of `host_tools` that specify the executable names and the flags for the tools used in the host toolchain.
* `add_as_target` can be specified that a target for this tool is being created. Optionally, the executable can be run with root rights, if `run_as_su` is set to true.
* `build_machine_build_targets` speficies the build targets for the build toolchain (this is currently needed but should be removed here and calculated in future.)


## Project and User Options
```JSON
{
    "settings": {
        "project": {
            "root": "../../..",
            "toolchain": "default",
            "build_directory": "build",
            "gendata_directory": "build-gen",
            "build_generator": "cmake",
            "build_generator_configuration": {
                "build_tool": "ninja",
                "parallel_build": "4"
            }
        }
    }
}
```
To decribe the content of the project settings briefly:
* The root of the project is in the `root` directory (relative to the settings file)
* The toolchain to be used is `default`
* The build directory is `build` (relative to the settings file)
* The gendata directory is `build-gen` (relative to the settings file)
* The `build_generator` and `build_generator_configuration` is currently not used, but shall be used in future to invoke the build by meta.

```JSON
{
    "settings": {
        "user": {
            "build_generator_configuration": {
                "build_configuration": "debug",
                "parallel_build": "24"
            }
        }
    }
}
```
To decribe the content of the user settings briefly:
* `build_generator_configuration` is currently not used, but shall be used in future to let the user overwrite the project settings to it's needs.


# Supported OS
Currently only `linux` is supported as host for the meta program and also the generated files can only be used on unix based systems. You might use it in Windows with WSL.

# Limitations
This software is in an very early stage. Lot of things are not working yet and there are a ton of things that can be improved, reworked.


# How to build serenity
```bash
cd serenity/DevTools/
git clone https://github.com/lnzero1dev/meta.git
cd meta/serenity
make
meta gen default-image
mkdir build
cmake ../build-gen && make -j$(nproc)
make build_image && make run
```
