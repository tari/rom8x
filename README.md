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

 * 0.3 - ???
    + Add support for 84CSE (84+ Color Silver Edition)
    + Updated build scripts (no longer use the obsolete TASM and devpac8x)
    + Revised documentation
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
this program is not responsible for any issues that arise with your (mis)use of
this program.
