# ch2inc
The following application is a dynamic converter from C header files to Assembler include files.

It's primary purpose was to replace the tool "h2inc.exe" used in the [BRender](https://github.com/lakor64/dsbrender) pipeline to achieve cross-platform host.h generation for other assembler than MASM.

## Dependencies
- C++20 compiler
- vcpkg
- cxxopts
- libclang (Clang C API)

## Building
Setup the environment variable "LLVM_ROOT" to the path of your LLVM root directory (eg.: C:\Program Files\LLVM)

Configure the cmake project as explained in the [vcpkg](https://vcpkg.io/en/getting-started.html) page.

Build the application with the toolset you prefer.

## Usage
`ch2inc.exe -d (driver name) -p (platform name) -b (bit size) (input file) (output file)`


For example, if you wish to generate an assembly file for Windows 32-bit MASM (for a codebase built with MSVC) you would run:

`ch2inc.exe -d ch2drvmasm -p win -b 32 --msvc host.h host.inc`

For a detailed information of the command line, see the help istructions in the program (`ch2inc.exe -h`).

## Supported drivers

### MASM
This is the current primary target as it can be used to verify the correctness of the application

### NASM
An entry-stage old import of the NASM driver with the old (and crappy) architecture was pushed, that can be used for reference. Keep in mind that:

1. NASM doesn't properly support nested structures, the generator needs to be aware of this and apply proper fixes
2. NASM doesn't have a sane way to do bitwise and it's very limited to get it properly
3. NASM doesn't have any real typing in structures or so on so you cannot expect to detect issues when trying to access for example a REAL8

