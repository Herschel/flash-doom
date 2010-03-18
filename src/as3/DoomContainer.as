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
 *  Main app class, wrapper around preloader + game manager
 *  AUTHOR: Mike Welsh
 *-----------------------------------------------------------------------------
 */

package 
{
	import flash.display.MovieClip;
	import flash.display.Sprite;
	import flash.display.StageQuality;
	import flash.events.Event;
	import flash.utils.getDefinitionByName;

	public class DoomContainer extends Sprite 
	{		
		private var _preloader:Sprite;
		private var _doomMenu:Sprite;
		private var _doomGame:Sprite;

		public function DoomContainer()
		{
			_preloader = new DoomPreloader();
			
			_preloader.addEventListener( Event.COMPLETE, loadingComplete );
			
			addChild( _preloader );
		}

		private function loadingComplete( e:Event ):void
		{
			removeChild( _preloader );
			_preloader = null;
			
			stage.quality = StageQuality.LOW;

			showMenu();
		}
		
		private function showMenu():void
		{
			var DoomMenuClass:Class = Class( getDefinitionByName("DoomMenuImpl") );
			_doomMenu = new DoomMenuClass() as Sprite;
			_doomMenu.addEventListener(Event.COMPLETE, startGameHandler);
			
			addChild( _doomMenu );
		}
		
		private function startGame(gameType:String):void
		{
			var DoomGameClass:Class = Class( getDefinitionByName("DoomGame") );
			_doomGame = Sprite( new DoomGameClass(gameType) );
			_doomGame.addEventListener(Event.COMPLETE, gameFinishedHandler);
			
			addChild(_doomGame);
		}
		
		private function startGameHandler( e:Event ):void
		{
			removeChild(_doomMenu);
			var gameType:String = Object(_doomMenu).gameType;
			_doomMenu = null;
			startGame(gameType);
		}
		
		private function gameFinishedHandler( e:Event ):void
		{
			removeChild(_doomGame);
			_doomGame = null;
			showMenu();
		}
	}
	
}