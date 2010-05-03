
_Copyright (c) 2006 By Duncan Mac-Vicar P._

Many thanks to  [Michael Mac-Vicar P.][2] for the invaluable help reverse engineering the ugly database format.

See License section.

## Introduction ##

I bought a My Mini Dj, nice player, ogg vorbis support, but only Windows software. And even the Windows software included is total *crap*.

The problem is that the music database file format is proprietary and undocumented so with the help of my [brother Michael][2] we figured out the binary database format, and then a couple of hacking nights were enough to make a test implementation. Note, the implementation was done just to get something working, I am aware the code is not something I am proud of.

This is only a proof of concept command line indexer so you can use all the features under Linux like browsing by artist, album, genre, etc instead of browsing the device as folders and files.

Probably it is easy to adapt to a MacOSX system.

## Download

For now, the source is available via subversion:

svn co https://svn.berlios.de/svnroot/repos/minidj

You can [browse the repository here.][3]

Probably you need to fix the Makefile to fit your system.

## Requirements

  * mtools (probably available for your Linux distribution as a package) You need to copy the files with mtools so they get copied with short names. The indexer need to know the right fat short name, and I havent figured another way to do it.
  * [Taglib][4], a audio metadata reader library from my friend [Scott Wheeler][5]. You need this library because the indexer uses it to extract the music metadata (artist, album, etc).
  * Qt installed, with the development headers so you can compile the program.

## Usage:

  * copy your files with mtools to /mymusic/k2
  * mount as msdos to /somemountpoint
  * run ./dj-index /somemountpoint
  * umount

## To Do

  * Fix lot of bugs. There are lot of headers in the database I just ignore because I have no clue what they mean.
  * Integrate it with amaroK?

## License

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

   [1]: http://img59.imageshack.us/img59/391/83090017fp.jpg
   [2]: http://michael.mac-vicar.com
   [3]: http://svn.berlios.de/wsvn/minidj
   [4]: http://developer.kde.org/~wheeler/taglib.html
   [5]: http://developer.kde.org/~wheeler

