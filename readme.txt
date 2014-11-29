-----------------------------------------------------------------------------
  FlashDoom
 
  based on Linux DOOM 1.10
  Copyright (C) 1999 by
  id Software

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.
-----------------------------------------------------------------------------

  
Requires the Adobe Alchemy Toolkit preview.
Specifically, the ALCHEMY_HOME variable should point to your Alchemy
directory.

A simple port of LinuxDoom to ActionScript bytecode via Alchemy,
with hooks in place to pass the frame and sound buffers to Flash.

TODO:
- Lots of silliness and stupidity in the game choice screen
- Instead of just copying + pasting the Heretic and Hexen codebases,
    unify support for all games into a single codebase
- Switch to a smarter source port, such as PrBoom+

UPDATE 11/29/2014
Added support for mouse-locking and interactive full-screen mode which were
added to the Flash Player over the past many versions.
Also added support for binding a key to a left mouse click, and cleaned up
the code a tiny bit.
This is probably my last and only update.
I won't bother to update the C code and tooling to use FlasCC as opposed to
the Alchemy beta. The old Alchemy generated SWCs still seem to work.