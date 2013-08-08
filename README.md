# Building

If you wish to build rom8x from source, there are two components that must be
built separately: the PC-side application that joins dumped ROM pages with an
OS file, and the calculator program that dumps the ROM pages.

Familiarity with the concepts of compiling software is assumed throughout this
process. For most users, the pre-compiled binaries provided with distribution
packages should be sufficient.

## PC application

Compiling the PC-side application is up to your discretion due to the
wide variety of platforms it should run on. For example, on a typical
Linux system:

    $ gcc -o rom8x rom8x.c

Or perhaps to cross-compile for a Windows target:

    $ i486-mingw32-gcc -o rom8x.exe rom8x.c

## Calculator programs

Similar to the PC application, assembling the calculator programs may
require some manual configuration. However, two automated build scripts
are provided for Windows and UNIX-like systems.

The build scripts assume you have [SPASM](https://wabbit.codeplex.com/)
on your PATH, which should work for both \*NIX and Windows users. They
will emit .8xp files for each supported calculator into
correspondingly-name directories.

Thus, the following on a \*NIX system generates 8xp files in five
subdirectories of the current directory:

    $ sh build.sh

And functionality is the same using the Windows batch processor:

    C:\>build.bat

### Manual assembly

If you can't or don't want to use the provided build scripts, manually
building binaries is a fairly straightforward process. You must define
`progName`, `pageNum` and `bootPage` appropriate for the target
calculator model. Refer to the build scripts to see what these
"approriate" values are-- they are specified on the assembler command
line as `-d<name>=<value>`.

For example, the configuration for a standard 83+ (83PBE) might be as follows:

    #define progName "83PBE"
    #define pageNum "1"
    #define boodPage 1Fh

# Credits

# License
