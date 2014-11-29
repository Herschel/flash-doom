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
	import cmodule.heretic.CLibInit;
	import cmodule.hexen.CLibInit;
	import flash.utils.Dictionary;
	
	import com.newgrounds.NewgroundsAPI;
	
	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.display.BlendMode;
	import flash.display.Sprite;
	import flash.display.StageDisplayState;
	import flash.events.Event;
	import flash.events.FullScreenEvent;
	import flash.events.MouseEvent;
	import flash.events.KeyboardEvent;
	import flash.events.SampleDataEvent;
	import flash.media.Sound;
	import flash.media.SoundChannel;
	import flash.net.navigateToURL;
	import flash.net.SharedObject;
	import flash.net.URLRequest;
	import flash.text.TextFormatAlign;
	import flash.text.TextField;
	import flash.text.TextFormat;
	import flash.text.TextFieldAutoSize;
	import flash.ui.Keyboard;
	import flash.ui.Mouse;
	import flash.utils.ByteArray;
	import flash.utils.getTimer;

	public class DoomGame extends Sprite
	{		
		private var _bitmap:Bitmap;
		private var _bData:BitmapData;
		
		private var _soundBuffer:ByteArray;
		private var _sound:Sound;
		private var _soundChannel:SoundChannel;
		
		private var _wadFile:ByteArray;
		
		private var _gameType:String;
		
		private var _lib:Object;
		private var _ram:ByteArray;
		
		private var _settings:DoomSettings;
		
		private static const KEY_FORWARD:uint = 0;
		private static const KEY_BACKWARD:uint = 1;
		private static const KEY_TURNRIGHT:uint = 2;
		private static const KEY_TURNLEFT:uint = 3;
		private static const KEY_FIRE:uint = 4;
		private static const KEY_USE:uint = 5;
		private static const KEY_STRAFELEFT:uint = 6;
		private static const KEY_STRAFERIGHT:uint = 7;
		private static const KEY_STRAFE:uint = 8;
		private static const KEY_RUN:uint = 9;
		private static const KEY_INVENTORY_LEFT:uint = 10;
		private static const KEY_INVENTORY_RIGHT:uint = 11;
		private static const KEY_USE_ITEM:uint = 12;
		private static const KEY_JUMP:uint = 13;
		private static const NUM_KEYS:uint = 14;

		private var _keys:Vector.<Boolean>;
		private var _keyBindings:Array;
		
		private var _dragging:Boolean = false;
		private var _lastMouseX:Number;
		private var _lastMouseY:Number;

		private var _mouseDx:Number;
		private var _mouseDy:Number;
		private var _mouseKeyCode:uint;
		
		private var _saveGames:SharedObject;
		
		private static const MOUSE_SCALE:uint = 10;
		
		private var _time:uint;
		private var _frameAccumulator:Number = 0;
		private var _frames:uint;
		private var _frameTime:uint;
		
		private var _quitting:Boolean = false;
		
		private static const NGAPI_MOVIE_ID:String = "5831";
		private static const NGAPI_ENCRYPT_KEY:String = "K5EUDyrsPmUfpE5z39Sbp7OwVYAes8vr";
			
		private var _medalNameMap:Array = ["Find Some Meat", "E1M9", "Where's Episode 2?", "Cockadoodledoo", "The Graveyard", "Blasphemer", "Master Fighter", "Master Cleric", "Master Mage"];
		private var _medalPopup:NewgroundsAPIMedalPopup;

		private var _fullScreenText:Sprite;
		private var _fullScreenTextTimer:Number;
		
		[Embed(source = "../../data/doom1.wad", mimeType = "application/octet-stream")]
		private static const DoomWadClass:Class;
		[Embed(source = "../../data/heretic1.wad", mimeType = "application/octet-stream")]
		private static const HereticWadClass:Class;
		[Embed(source = "../../data/hexen.wad", mimeType = "application/octet-stream")]
		private static const HexenWadClass:Class;
		
		private static const SCREEN_WIDTH:uint = 320;
		private static const SCREEN_HEIGHT:uint = 200;
		
		private static const GAME_FRAMERATE:uint = 35;
		private static const MAX_FRAME_SKIP:uint = 5;
		
		private static const GAME_SAMPLES:uint = 315*4;	// 44100KHz / 35 fps
		private static const FLASH_SAMPLES:uint = 2048;
		private static const CHANNELS:uint = 2;
		private static const SAMPLE_SIZE:uint = 4;
		private static const GAME_SOUNDBUFF_SIZE:uint = GAME_SAMPLES * CHANNELS * SAMPLE_SIZE;
		private static const FLASH_SOUNDBUFF_SIZE:uint = FLASH_SAMPLES * CHANNELS * SAMPLE_SIZE;
		
		public static const DOOM:String		= "Doom";
		public static const HERETIC:String	= "Heretic";
		public static const HEXEN:String	= "Hexen";
		
		public function DoomGame(gameType:String)
		{
			_gameType = gameType;
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
			
			_mouseDx = _mouseDy = 0;

			_settings = new DoomSettings();
			_keyBindings = _settings.keyBindings;
			_mouseKeyCode = _settings.mouseKeyCode;

			_keys = new Vector.<Boolean>(256, true);
			for (i = 0; i < 256; i++)
				_keys[i] = false;

			var libInit:Object;
			var gameClass:Class;
			switch(_gameType)
			{
				case DOOM:
					_wadFile = new DoomWadClass() as ByteArray;
					gameClass = cmodule.doom.CLibInit;
					libInit = new gameClass();
					libInit.putEnv("HOME", ".");
					libInit.supplyFile("./doom1.wad", _wadFile);
					
					_saveGames = SharedObject.getLocal("DoomSaves");
					if (!_saveGames.data.saves)
					{
						_saveGames.data.saves = new Array();
						for (i = 0; i < 6; i++)
						{
							_saveGames.data.saves[i] = new ByteArray();
						}
						
					}
					break;
				case HERETIC:
					_wadFile = new HereticWadClass() as ByteArray;
					gameClass = cmodule.heretic.CLibInit;
					libInit = new gameClass();
					libInit.supplyFile("heretic1.wad", _wadFile);
					_saveGames = SharedObject.getLocal("HereticSaves");
					if (!_saveGames.data.saves)
					{
						_saveGames.data.saves = new Array();
						for (i = 0; i < 6; i++)
						{
							_saveGames.data.saves[i] = new ByteArray();
						}
						
					}
					break;
				case HEXEN:
					_wadFile = new HexenWadClass() as ByteArray;
					gameClass = cmodule.hexen.CLibInit;
					libInit = new gameClass();
					libInit.supplyFile("hexen.wad", _wadFile);
					_saveGames = SharedObject.getLocal("HexenSaves");
					if (!_saveGames.data.saves)
					{
						_saveGames.data.saves = new Dictionary();
						
					}
					break;
			}
			
			_lib = libInit.init();

			_ram = _lib.getRam();
			_lib.setThiz( this );
			
			_lib.setKeyBindings( _keyBindings );
			
			buttonMode = true;
			useHandCursor = true;
			
			_medalPopup = new NewgroundsAPIMedalPopup();
			addChild(_medalPopup);

			addEventListener( Event.ADDED_TO_STAGE, addedToStageHandler );
		}
		
		private function addedToStageHandler(e:Event):void
		{
			removeEventListener( Event.ADDED_TO_STAGE, addedToStageHandler );
			
			stage.addEventListener( KeyboardEvent.KEY_DOWN, keyDownHandler );
			stage.addEventListener( KeyboardEvent.KEY_UP, keyUpHandler );
			
			stage.frameRate = 65;
			stage.focus = stage;
			
			var scale:Number = Math.min(stage.stageWidth/SCREEN_WIDTH, stage.stageHeight/SCREEN_HEIGHT);
			_bitmap.scaleX = _bitmap.scaleY = scale;
			
			stage.addEventListener(Event.ENTER_FRAME, enterFrameHandler);
			stage.addEventListener(MouseEvent.MOUSE_DOWN, mouseDownHandler);
			stage.addEventListener(MouseEvent.MOUSE_UP, mouseUpHandler);
			stage.addEventListener(Event.MOUSE_LEAVE, mouseUpHandler);
			stage.addEventListener(MouseEvent.MOUSE_MOVE, mouseMoveHandler);
			stage.addEventListener(FullScreenEvent.FULL_SCREEN, fullScreenHandler);
			stage.addEventListener(FullScreenEvent.FULL_SCREEN_INTERACTIVE_ACCEPTED, fullScreenAcceptedHandler);
			addEventListener(Event.DEACTIVATE, deactivateHandler);

			NewgroundsAPI.connectMovie(this, NGAPI_MOVIE_ID, NGAPI_ENCRYPT_KEY, false);
			NewgroundsAPI.loadMedals();
			
			_medalPopup.x = -2048;
			
			toggleFullScreen();

			var fullScreenTextField:TextField = new TextField();
			fullScreenTextField.multiline = false;
			fullScreenTextField.selectable = false;
			fullScreenTextField.autoSize = TextFieldAutoSize.CENTER;
			var textFormat:TextFormat = new TextFormat();
			textFormat.align = TextFormatAlign.CENTER;
			textFormat.color = 0xffffffff;
			textFormat.size = 24;
			textFormat.bold = true;
			fullScreenTextField.x = (stage.stageWidth - fullScreenTextField.width)/2;
			fullScreenTextField.y = 2;
			fullScreenTextField.defaultTextFormat = textFormat;
			fullScreenTextField.text = "Press Alt + Enter to re-enter full screen mode.";

			_fullScreenText = new Sprite();
			_fullScreenText.addChild( fullScreenTextField );
			_fullScreenText.graphics.beginFill( 0, 0756 );
			_fullScreenText.graphics.drawRect( 0, 0, stage.stageWidth, fullScreenTextField.height + 4 );
			_fullScreenText.graphics.endFill();
			_fullScreenText.y = 100;
			_fullScreenText.visible = false;
			_fullScreenText.blendMode = BlendMode.LAYER;
			addChild(_fullScreenText);

			_frames = 0;
			_time = getTimer();
		}
		
		private function toggleFullScreen():void
		{
			if(stage)
			{
				if(stage.displayState == StageDisplayState.NORMAL )
				{
					try
					{
						stage.displayState = StageDisplayState.FULL_SCREEN_INTERACTIVE;
					}
					catch(error:*) { }
				}
				else
				{
					stage.displayState = StageDisplayState.NORMAL;
					isMouseLocked = false;
				}
			}
		}

		private function fullScreenHandler(e:FullScreenEvent):void
		{
			if(_fullScreenText)
			{
				if(!e.fullScreen)
				{
					_fullScreenText.visible = true;
					_fullScreenTextTimer = 5.0;
				}
				else
				{
					_fullScreenText.visible = false;
				}
			}
		}

		private function fullScreenAcceptedHandler(e:FullScreenEvent):void
		{
			isMouseLocked = true;
		}

		private function set isMouseLocked(v:Boolean):void
		{
			try
			{
				stage.mouseLock = v;
				if(stage.mouseLock)
				{
					Mouse.hide();
				}
			}
			catch(error:*) { }
		}

		private function get isMouseLocked():Boolean
		{
			try
			{
				return stage.mouseLock;
			}
			catch(error:*) { }
			return false;
		}

		// GAME TICK
		private function enterFrameHandler(e:Event):void
		{
			var ptr:uint;
			var i:uint;
			
			var dt:uint = getTimer()-_time;
			_time += dt;
			
			_frameAccumulator += dt;
			
			i = 0;
			while (i<MAX_FRAME_SKIP && _frameAccumulator >= 1000 / GAME_FRAMERATE)
			{
				if(stage)
				{
					if( isMouseLocked )
					{
						_lib.mouseMove( _mouseDx*MOUSE_SCALE, _mouseDy*MOUSE_SCALE );
						_mouseDx = _mouseDy = 0;
					}
					else
					{
						if(_dragging)
						{
							var dx:int = stage.mouseX - _lastMouseX;
							var dy:int = stage.mouseY - _lastMouseY;
							_lib.mouseMove( dx*MOUSE_SCALE, dy*MOUSE_SCALE );
							_lastMouseX = stage.mouseX;
							_lastMouseY = stage.mouseY;
						}
					}
				}

				_lib.tick();

				// If not mouse-locked, we have to allow dragging to simulate mouse-lock.
				// Therefore, only fire click binding for one frame.
				if(!isMouseLocked && _dragging && _mouseKeyCode && !_keys[_mouseKeyCode])
				{
					_lib.keyUp(_mouseKeyCode);
				}
							
				ptr = _lib.getSoundData();
				_soundBuffer.writeBytes( _ram, ptr, GAME_SOUNDBUFF_SIZE );
				
				i++;
				_frameAccumulator -= 1000/GAME_FRAMERATE;
			}
			
			if (i == MAX_FRAME_SKIP)_frameAccumulator = 0; // prevent spirals of death
			
			// if we ticked, then update display
			if (i > 0)
			{
				_lib.render();
				
				_bData.lock();
				ptr = _lib.getFrameBuffer();
				_ram.position = ptr;
				_bData.setPixels( _bData.rect, _ram );
				_bData.unlock();
			}
			
			if (!_soundChannel)
			{
				_soundChannel = _sound.play();
				_soundChannel.addEventListener(Event.SOUND_COMPLETE, soundCompleteHandler);
			}

			if( _fullScreenText && _fullScreenText.visible )
			{
				_fullScreenTextTimer -= dt / 1000;
				_fullScreenText.alpha = Math.min( Math.max( _fullScreenTextTimer, 0.0 ), 1.0 );
				if( _fullScreenText.alpha <= 0 )
				{
					_fullScreenText.visible = false;
				}

			}

			if (_quitting) doQuit();		// catch if game exited
		}
		
		private function sampleDataHandler(event:Event):void
		{
			var e:SampleDataEvent = SampleDataEvent(event);

			if (_soundBuffer.length >= FLASH_SOUNDBUFF_SIZE )
			{
				e.data.writeBytes(_soundBuffer, 0, FLASH_SOUNDBUFF_SIZE);
				_soundBuffer.position = 0;
				_soundBuffer.writeBytes(_soundBuffer, FLASH_SOUNDBUFF_SIZE);
				_soundBuffer.length -= FLASH_SOUNDBUFF_SIZE;
			} 
			else
			{
				for (var i:uint = 0; i < FLASH_SOUNDBUFF_SIZE; i+=4)
					e.data.writeFloat(0);
			}
			
			// prevent too much lag in sound
			if (_soundBuffer.length >= FLASH_SOUNDBUFF_SIZE*2.5)
			{
				_soundBuffer.position = 0;
				_soundBuffer.writeBytes( _soundBuffer, _soundBuffer.length-FLASH_SOUNDBUFF_SIZE);
				_soundBuffer.length = FLASH_SOUNDBUFF_SIZE;
			}
		  
		}
		
		
		// INPUT
		
		private function keyDownHandler(e:KeyboardEvent):void
		{
			if(e.altKey && e.keyCode == Keyboard.ENTER)
			{
				toggleFullScreen();
				return;
			}

			if(_lib && !_keys[e.keyCode])
			{
				_lib.keyDown( e.keyCode );
			}
			_keys[e.keyCode] = true;
		}
		
		private function keyUpHandler(e:KeyboardEvent):void
		{
			if (_lib && _keys[e.keyCode])
			{
				_lib.keyUp( e.keyCode );
			}
				
			_keys[e.keyCode] = false;
		}
		
		private function mouseDownHandler(e:MouseEvent):void
		{
			_dragging = true;
			Mouse.hide();
			if(stage)
			{
				_lastMouseX = stage.mouseX;
				_lastMouseY = stage.mouseY;
			}

			// Clicking also activates a button key
			if(_mouseKeyCode)
				_lib.keyDown(_mouseKeyCode);
		}
		
		private function mouseUpHandler(e:Event):void
		{
			if(!isMouseLocked)
			{
				Mouse.show();
			}
			else if( _mouseKeyCode && !_keys[_mouseKeyCode])
			{
				_lib.keyUp(_mouseKeyCode);
			}

			_dragging = false;
		}

		private function deactivateHandler(e:Event):void
		{
			Mouse.show();
			_dragging = false;
			for(var i:int=0; i<256; ++i)
			{
				if(_keys[i])
				{
					_keys[i] = false;
					_lib.keyUp(i);
				}
			}
		}
		
		private function mouseMoveHandler(e:MouseEvent):void
		{
			try
			{
				_mouseDx += e.movementX;
				_mouseDy += e.movementY;
			}
			catch(error:*) { }
		}
		
		// C++ hooks into Flash
		
		public function getSaveGame(i:*):ByteArray
		{
			if (!_saveGames)
				return new ByteArray();
				
			 if (!_saveGames.data.saves[i])
				_saveGames.data.saves[i] = new ByteArray();
			
			_saveGames.data.saves[i].position = 0;
			return _saveGames.data.saves[i];
		}		

		public function goToURL(url:String):void
		{
			navigateToURL(new URLRequest(url), "_blank");
		}
		
		private function soundCompleteHandler(e:Event):void
		{
			// insist that our sound restarts next frame
			_soundChannel.removeEventListener(Event.SOUND_COMPLETE, soundCompleteHandler);
			_soundChannel = null;
		}
		
		public function awardMedal(medalId:uint):void
		{			
			if(_medalPopup)
			{
				_medalPopup.x = (stage.stageWidth - _medalPopup.width)/2;
				_medalPopup.y = 32;
			}
			NewgroundsAPI.unlockMedal(_medalNameMap[medalId]);
		}
		
		public function quitGame():void
		{
			_quitting = true;
		}
		
		public function doQuit():void
		{
			_bData.dispose();
			_bData = null;
			
			removeChild(_bitmap);
			
			removeEventListener(SampleDataEvent.SAMPLE_DATA, sampleDataHandler);
			stage.removeEventListener(Event.ENTER_FRAME, enterFrameHandler);
			stage.removeEventListener( KeyboardEvent.KEY_DOWN, keyDownHandler );
			stage.removeEventListener( KeyboardEvent.KEY_UP, keyUpHandler );
			stage.removeEventListener(MouseEvent.MOUSE_DOWN, mouseDownHandler);
			stage.removeEventListener(MouseEvent.MOUSE_UP, mouseUpHandler);
			stage.removeEventListener(Event.MOUSE_LEAVE, mouseUpHandler);
			stage.removeEventListener(MouseEvent.MOUSE_MOVE, mouseMoveHandler);
			stage.removeEventListener(FullScreenEvent.FULL_SCREEN, fullScreenHandler);
			stage.removeEventListener(FullScreenEvent.FULL_SCREEN_INTERACTIVE_ACCEPTED, fullScreenAcceptedHandler);
			removeEventListener(Event.DEACTIVATE, deactivateHandler);

			if (_soundChannel)
			{
				_soundChannel.stop();
				_soundChannel = null;
			}
			
			if (_medalPopup)
			{
				removeChild(_medalPopup);
				_medalPopup = null;
			}

			_saveGames.flush();
			_saveGames = null;
			
			_settings.flush();
			_settings = null;
			
			_lib = null;
			_ram = null;
			
			_soundBuffer.clear();
			_soundBuffer = null;
			
			_wadFile.clear();
			_wadFile = null;
			
			dispatchEvent(new Event(Event.COMPLETE));
			
		}
	}
	
}