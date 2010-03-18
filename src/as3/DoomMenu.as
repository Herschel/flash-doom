package 
{
	import flash.display.MovieClip;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.KeyboardEvent;
	import flash.events.MouseEvent;
	import flash.net.SharedObject;
	import flash.text.TextField;
	import flash.ui.Keyboard;
		
	public class DoomMenu extends MovieClip 
	{
		// in DoomAssets.swc
		public var _doomButton:Sprite;
		public var _hereticButton:Sprite;
		public var _hexenButton:Sprite;
		public var _controlsButton:Sprite;
		public var _backButton:Sprite;
		public var _forwardText:Sprite;
		public var _backwardText:Sprite;
		public var _turnLeftText:Sprite;
		public var _turnRightText:Sprite;
		public var _fireText:Sprite;
		public var _useText:Sprite;
		public var _runText:Sprite;
		public var _strafeLeftText:Sprite;
		public var _strafeRightText:Sprite;
		public var _inventoryLeftText:Sprite;
		public var _inventoryRightText:Sprite;
		public var _inventoryUseText:Sprite;
		public var _jumpText:Sprite;
		
		private var _gameType:String;

		private var _saveData:SharedObject;
		private var _keyBindings:Array;
		private var _keyTexts:Array;
		
		private var _selectedKey:uint;
		
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
		
		public function DoomMenu():void
		{
			stop();
			
			initGameMenu();
		}
		
		private function initGameMenu():void
		{
			gotoAndStop(1);
			if (_doomButton)
			{
				_doomButton.addEventListener(MouseEvent.CLICK, gameClickedHandler, false, 0, true);
				_hereticButton.addEventListener(MouseEvent.CLICK, gameClickedHandler, false, 0, true);
				_hexenButton.addEventListener(MouseEvent.CLICK, gameClickedHandler, false, 0, true);
				_doomButton.buttonMode = true;
				_hereticButton.buttonMode = true;
				_hexenButton.buttonMode = true;
				_controlsButton.buttonMode = true;
				
				_controlsButton.addEventListener(MouseEvent.CLICK, controlsClickedHandler, false, 0, true);
			}
		}
		
		private function controlsClickedHandler(e:MouseEvent):void
		{
			gotoAndStop(2);
			initControlsMenu();
		}
		
		private function gameClickedHandler(e:MouseEvent):void
		{
			var gameType:String;
			switch(e.target)
			{
				case _doomButton:		_gameType = "Doom";		break;
				case _hereticButton:	_gameType = "Heretic";	break;
				case _hexenButton:		_gameType = "Hexen";	break;
			}
			
			dispatchEvent(new Event(Event.COMPLETE));
		}
		
		public function get gameType():String	{ return _gameType; }
		
		public function initControlsMenu():void
		{
			_keyTexts =
				[_forwardText, _backwardText, _turnRightText, _turnLeftText, _fireText, _useText, _strafeLeftText, _strafeRightText, null,
				_runText, _inventoryLeftText, _inventoryRightText, _inventoryUseText, _jumpText];
				
			_saveData = SharedObject.getLocal("DoomTriplePack");
			_keyBindings = _saveData.data.keyBindings;

			if(!_keyBindings)
			{
				_keyBindings = new Array(NUM_KEYS);
				
				for (i = 0; i < NUM_KEYS; i++)
					_keyBindings[i] = 0;
				
				_keyBindings[KEY_FORWARD] = 87;
				_keyBindings[KEY_BACKWARD] = 83;
				_keyBindings[KEY_FIRE] = Keyboard.SPACE;
				_keyBindings[KEY_USE] = 82;
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
			
			for (var i:uint = 0; i < _keyTexts.length; i++)
			{
				if (_keyTexts[i])
				{
					_keyTexts[i].text.text = keyString(uint(_keyBindings[i]));
					_keyTexts[i].addEventListener(MouseEvent.CLICK, changeBindingHandler, false, 0, true);
					_keyTexts[i].buttonMode = true;
					_keyTexts[i].mouseChildren = false;
				}
			}
			
			_backButton.addEventListener(MouseEvent.CLICK, backClickedHandler, false, 0, true);
			_backButton.buttonMode = true;
		}
		
		private function backClickedHandler(e:MouseEvent):void
		{
			_saveData.flush();
			if (stage.hasEventListener(KeyboardEvent.KEY_DOWN))
				stage.removeEventListener(KeyboardEvent.KEY_DOWN, keyDownHandler);
				
			_saveData = null;
			
			initGameMenu();
		}
		
		private function changeBindingHandler(e:MouseEvent):void
		{
			for (var i:uint = 0; i < _keyTexts.length; i++)
				if (_keyTexts[i] == e.target)
					break;
			
			stage.addEventListener(KeyboardEvent.KEY_DOWN, keyDownHandler, false, 0, true);
			_selectedKey = i;
			_keyTexts[i].text.text = "press a key";
		}
		
		private function keyDownHandler(e:KeyboardEvent):void
		{
			if (e.keyCode == Keyboard.ESCAPE) return;
			if (e.keyCode >= 0x30 && e.keyCode <= 0x39) return;
			
			_keyBindings[_selectedKey] = e.keyCode;
			_keyTexts[_selectedKey].text.text = keyString(e.keyCode);
			
			stage.removeEventListener(KeyboardEvent.KEY_DOWN, keyDownHandler);
		}
		
		private function keyString(key:uint):String
		{
			if(key>=0x30 && key<=0x5a)
				return String.fromCharCode(key).toLowerCase();
			
			switch(key)
			{
				case 0:					return "none";
				case Keyboard.DOWN:		return "down";
				case Keyboard.UP:		return "up";
				case Keyboard.LEFT:		return "left";
				case Keyboard.RIGHT:	return "right";
				case Keyboard.ENTER:	return "enter";
				case Keyboard.SHIFT:	return "shift";
				case 18:				return "alt";
				case Keyboard.CONTROL:	return "control";
				case Keyboard.BACKSPACE:return "backspace";
				case Keyboard.TAB:		return "tab";
				case Keyboard.CAPS_LOCK:return "capslock";
				case Keyboard.DELETE:	return "delete";
				case 219:				return "[";
				case 221:				return "]";
				case Keyboard.SPACE:	return "space";
				default:				return "key " + key.toString();
			}
		}
	}
	
}