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
	import cmodule.doom.CLibInit;
	import cmodule.doom.MemUser;
	
	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.KeyboardEvent;
	import flash.events.SampleDataEvent;
	import flash.media.Sound;
	import flash.media.SoundChannel;
	import flash.net.navigateToURL;
	import flash.net.SharedObject;
	import flash.net.URLRequest;
	import flash.utils.ByteArray;

	public class DoomGame extends Sprite
	{		
		private var _bitmap:Bitmap;
		private var _bData:BitmapData;
		private var _memUser:Object;
		
		private var _soundBuffer:ByteArray;
		private var _soundBufferPos:uint = 0;
		private var _sound:Sound;
		private var _soundChannel:SoundChannel;
		
		[Embed(source = "../../bin/doom1.wad", mimeType = "application/octet-stream")]
		private static const DoomWadClass:Class;
		private var _doomWad:ByteArray;
		
		private var _lib:Object;
		private var _ram:ByteArray;
		
		private var _saveData:SharedObject;
		
		private var _keys:Vector.<Boolean>;
		
		private static const SCREEN_WIDTH:uint = 320;
		private static const SCREEN_HEIGHT:uint = 200;
		
		private static const GAME_SAMPLES:uint = 1260;	// 2048 - 788 overlap from previous frame
		private static const GAME_SOUNDBUFF_OFFSET:uint = 2048 - GAME_SAMPLES;
		private static const FLASH_SAMPLES:uint = 2048;
		private static const FLASH_MIN_SAMPLES:uint = 2048;
		private static const CHANNELS:uint = 2;
		private static const SAMPLE_SIZE:uint = 4;
		private static const GAME_SOUNDBUFF_SIZE:uint = GAME_SAMPLES * CHANNELS * SAMPLE_SIZE;
		private static const FLASH_SOUNDBUFF_SIZE:uint = FLASH_SAMPLES * CHANNELS * SAMPLE_SIZE;
		private static const FLASH_MIN_SOUNDBUFF_SIZE:uint = FLASH_MIN_SAMPLES * CHANNELS * SAMPLE_SIZE;
		
		public function DoomGame()
		{
			initGame();
		}
		
		private function initGame():void
		{
			var i:uint;
			
			_bData = new BitmapData(320, 200, false, 0);
			_bitmap = new Bitmap(_bData);
			addChild(_bitmap);
			
			_soundBuffer = new ByteArray();
			_sound = new Sound();
			_sound.addEventListener( SampleDataEvent.SAMPLE_DATA, sampleDataHandler );
			
			_saveData = SharedObject.getLocal("Doom");
			if (!_saveData.data.saveGames)
			{
				_saveData.data.saveGames = new Array();
				for (i = 0; i < 6; i++)
					_saveData.data.saveGames[i] = new ByteArray();
			}
			
			_keys = new Vector.<Boolean>(256, true);
			for (i = 0; i < 256; i++)
				_keys[i] = false;
				
			_doomWad = new DoomWadClass() as ByteArray;
			
			var _libInit:CLibInit = new CLibInit();
			_libInit.putEnv("HOME", ".");
			_libInit.supplyFile("./doom1.wad", _doomWad);
			_lib = _libInit.init();
						
			_ram = _lib.getRam();
			_lib.setThiz( this );
			
			_memUser = new MemUser();
			
			addEventListener( Event.ADDED_TO_STAGE, addedToStageHandler );
		}
		
		private function addedToStageHandler(e:Event):void
		{
			removeEventListener( Event.ADDED_TO_STAGE, addedToStageHandler );
			
			stage.addEventListener( KeyboardEvent.KEY_DOWN, keyDownHandler );
			stage.addEventListener( KeyboardEvent.KEY_UP, keyUpHandler );
			stage.frameRate = 35;
			stage.focus = stage;
			
			var scale:Number = Math.min(stage.stageWidth/SCREEN_WIDTH, stage.stageHeight/SCREEN_HEIGHT);
			_bitmap.scaleX = _bitmap.scaleY = scale;
			
			addEventListener(Event.ENTER_FRAME, enterFrameHandler);
		}
		
		private function enterFrameHandler(e:Event):void
		{
			var ptr:uint;
			var i:uint, j:uint;
			
			_bData.lock();
			_lib.tick();
						
			ptr = _lib.getSoundData() + (GAME_SOUNDBUFF_OFFSET * CHANNELS * SAMPLE_SIZE);
			_soundBuffer.writeBytes( _ram, ptr, GAME_SOUNDBUFF_SIZE );
			
			if (!_soundChannel)
			{
				_soundChannel = _sound.play();
				_soundChannel.addEventListener(Event.SOUND_COMPLETE, soundCompleteHandler);
			}
			
			ptr = _lib.getFrameBuffer();
			_ram.position = ptr;
			_bData.setPixels( _bData.rect, _ram );
			_bData.unlock();
		}
		
		private function sampleDataHandler(event:Event):void
		{
			var e:SampleDataEvent = SampleDataEvent(event);
			
			var l:uint = Math.min( FLASH_SOUNDBUFF_SIZE, _soundBuffer.length - _soundBufferPos  );
			if (l < FLASH_MIN_SOUNDBUFF_SIZE )
			{return;
				for (var i:uint = 0; i < FLASH_MIN_SAMPLES * CHANNELS; i++)
					e.data.writeFloat(0);
			}
			else
			{
				e.data.writeBytes(_soundBuffer, _soundBufferPos, l);
				_soundBufferPos += l;
				_soundBuffer.position = 0;
				_soundBuffer.writeBytes(_soundBuffer, _soundBufferPos);
				_soundBuffer.length = _soundBuffer.length - _soundBufferPos;
				_soundBufferPos = 0;
			}	
			
		}
		
		private function keyDownHandler(e:KeyboardEvent):void
		{
			if(_lib && !_keys[e.keyCode])
				_lib.keyDown( e.keyCode );
			_keys[e.keyCode] = true;
		}
		
		private function keyUpHandler(e:KeyboardEvent):void
		{
			if (_lib && _keys[e.keyCode])
				_lib.keyUp( e.keyCode );
				
			_keys[e.keyCode] = false;
		}
		
		public function getSaveGame(i:int, clear:Boolean):ByteArray
		{
			if (!_saveData.data.saveGames || !_saveData.data.saveGames[i])
				return new ByteArray();
			
			if (clear)_saveData.data.saveGames[i].clear();
			
			_saveData.data.saveGames[i].position = 0;
			return _saveData.data.saveGames[i];
		}

		public function goToNewgroundsGames():void
		{
			navigateToURL(new URLRequest("http://www.newgrounds.com/game"), "_blank");
		}
		
		private function soundCompleteHandler(e:Event):void
		{
			// insist that our sound restarts next frame
			_soundChannel.removeEventListener(Event.SOUND_COMPLETE, soundCompleteHandler);
			_soundChannel = null;
		}
	}
	
}