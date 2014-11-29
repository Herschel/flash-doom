/*-----------------------------------------------------------------------------
 *  FlashDoom
 * 
 *  based on Linux DOOM 1.10
 *  Copyright (C) 1999 by
 *  id Software
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 *  DESCRIPTION:
 *  Game manager, ticks & hooks into the Doom code
 *  AUTHOR: Mike Welsh
 *-----------------------------------------------------------------------------
 */

package
{
	import flash.net.SharedObject;
	import flash.ui.Keyboard;

	public class DoomSettings
	{
		public static const KEY_FORWARD:uint = 0;
		public static const KEY_BACKWARD:uint = 1;
		public static const KEY_TURNRIGHT:uint = 2;
		public static const KEY_TURNLEFT:uint = 3;
		public static const KEY_FIRE:uint = 4;
		public static const KEY_USE:uint = 5;
		public static const KEY_STRAFELEFT:uint = 6;
		public static const KEY_STRAFERIGHT:uint = 7;
		public static const KEY_STRAFE:uint = 8;
		public static const KEY_RUN:uint = 9;
		public static const KEY_INVENTORY_LEFT:uint = 10;
		public static const KEY_INVENTORY_RIGHT:uint = 11;
		public static const KEY_USE_ITEM:uint = 12;
		public static const KEY_JUMP:uint = 13;
		public static const NUM_KEYS:uint = 14;

		private var _keyBindings:Array;
		private var _saveData:SharedObject;

		public function get keyBindings():Array
		{
			return _keyBindings;
		}

		public function get mouseButton():int
		{
			return _saveData.data.mouseButton;
		}

		public function set mouseButton(v:int):void
		{
			_saveData.data.mouseButton = v;
		}

		public function get mouseKeyCode():uint
		{
			if(mouseButton >= 0)
			{
				return _keyBindings[mouseButton];
			}
			return 0;
		}

		public function DoomSettings()
		{
			_saveData = SharedObject.getLocal("DoomTriplePack1.1");

			_keyBindings = _saveData.data.keyBindings;
			if(!_keyBindings)
			{
				_keyBindings = new Array(NUM_KEYS);

				_keyBindings[KEY_FORWARD] = 87;
				_keyBindings[KEY_BACKWARD] = 83;
				_keyBindings[KEY_FIRE] = Keyboard.CONTROL;
				_keyBindings[KEY_USE] = Keyboard.SPACE;
				_keyBindings[KEY_TURNLEFT] = Keyboard.LEFT;
				_keyBindings[KEY_TURNRIGHT] = Keyboard.RIGHT;
				_keyBindings[KEY_RUN] = Keyboard.SHIFT;
				_keyBindings[KEY_STRAFELEFT] = 65;
				_keyBindings[KEY_STRAFERIGHT] = 68;
				_keyBindings[KEY_INVENTORY_LEFT] = 219;
				_keyBindings[KEY_INVENTORY_RIGHT] = 221;
				_keyBindings[KEY_USE_ITEM] = Keyboard.ENTER;
				_keyBindings[KEY_JUMP] = 81;
				_saveData.data.keyBindings = _keyBindings;
			}

			if(!_saveData.data.mouseButton)
				_saveData.data.mouseButton = KEY_FIRE;
		}

		public function flush():void
		{
			_saveData.data.keyBindings = _keyBindings;
			_saveData.flush();
		}
	}
}