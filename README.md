rom8x is a tool to create ROM files for TI-83+ series calculators which can
be used in emulators. It supports the following calculators.

 * TI-83+ (83PBE)
 * TI-83+ Silver Edition (83PSE)
 * TI-84+ (84PBE)
 * TI-84+ Silver Edition (84PSE)
 * TI-84+ Color Silver Edition (84CSE)

# Usage

The general process of building a ROM involves dumping some data from your
calculator, then combining that with a calculator OS upgrade file to create
a ROM.

Before beginning, determine which of the above models your calculator is, and
make note of the 5-character abbreviation that corresponds to it.  You will
also need a linking program such as TiLP or TI-Connect and an appropriate cable
to transfer files between your calculator and computer.

## Dumping

To dump the necessary data from your calculator, transfer the one or two 8xp
files provided with rom8x that correspond to your calculator model to the
calculator.  For example, if you have a TI-83+ use `G83PBE1.8xp`, or if you
have a TI-84+ Silver Edition use `G84PSE1.8xp` and `G84PSE2.8xp`.

Run the program you just put on your calculator. It will create a new AppVar in
RAM with name corresponding to the program's name. For example, the program
`G83PBE1` creates AppVar `D83PBE1`. Transfer this AppVar back to your computer
and place it in the same directory as rom8x.

If your calculator takes two programs (all 84+ models), repeat this process for
the second program.

## OS File

Now acquire an OS upgrade file for your device. These can be obtained from
[TI's web site](http://education.ti.com/)- find and download an OS file for
your calculator, and save the resultant 8xu or 8Cu (for the 84CSE) in the same
directory as rom8x.

## Combining

The final step involves running rom8x to combine all the files into a single
ROM. Open a terminal (the exact procedure depends on your operating system- on
Windows, open a Command Prompt) and `cd` to the directory containing your rom8x
files.

Now invoke rom8x with your calculator's model, and the -u option specifying the
name of the OS file you obtained. For example, for a 83PBE:

    rom8x 83PBE -u ti83plus_1.19.8xu

Or for a 84CSE:

    rom8x 84CSE -u Ti84Plus_OS.8Cu

The program automatically looks for AppVar files with names corresponding to
the specified calculator model. If you have renamed them, use the -1 and -2
options to specify their names as applicable.

    rom8x 84PBE -1 MyDump1.8xv -2 MyDump2.8xv -u 84pbe.8xu

If the process is successful, rom8x will create a new .rom file in your working
directory.

# Building rom8x

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
correspondingly-named directories.

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
    #define bootPage 1Fh

# Boilerplate

## Contact

Send questions, comments, etc to Peter Marheine: <peter@taricorp.net>.

## Version History

 * 0.3 - 20131007
    + Add support for 84CSE (84+ Color Silver Edition)
    + Updated build scripts (no longer use the obsolete TASM and devpac8x)
    + Revised documentation
    + Fixed various inefficiencies and security issues.
 * 0.2 - 20051209
    + Bug fix in G8?P?E programs
    + Completely recoded form scratch in C (v0.1 was in C++)
    + New, more flexible argument input method
    + Allows OS upgrade file integration into the ROM
    + Source! (released under the terms of the GNU GPL license)
 * 0.1 - 20050802
    + First Public Release

## Credits

rom8x is the work of Andree Chea, now maintained by Peter Marheine. Assistance
was provided by the following individuals:

 * Benjamin Moody (a.k.a. FloppusMaximus) for TilEm, OS Tools, and his
   invaluable support. :)
 * Tim Singer and Romain Liévin (and those who helped them) for making the
   TI-XX Link Protocol Guide.
 * Various testers for testing this package.

## License

Copyright (C) 2013 Peter Marheine <peter@taricorp.net>  
Copyright (C) 2005 Andree Chea <andree@ss.ticalc.org>  
Portions copyright (C) 2003 Benjamin Moody <benjamin@ecg.mit.edu>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA  02110-1301, USA.

ROM images are copyrighted by TI and should not be distributed.  The author of
this program is not responsible for any issues that arise from your (mis)use of
this program.
