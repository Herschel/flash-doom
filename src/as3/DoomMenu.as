package 
{
	import flash.display.MovieClip;
	import flash.display.Sprite;
	import flash.display.StageDisplayState;
	import flash.events.Event;
	import flash.events.KeyboardEvent;
	import flash.events.MouseEvent;
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

		private var _settings:DoomSettings;
		private var _keyTexts:Array;
		
		private var _selectedKey:int;
		
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

			_settings = new DoomSettings();

			_selectedKey = -1;
			
			for (var i:uint = 0; i < _keyTexts.length; i++)
			{
				if (_keyTexts[i])
				{
					_keyTexts[i].addEventListener(MouseEvent.CLICK, changeBindingHandler, false, 0, true);
					_keyTexts[i].buttonMode = true;
					_keyTexts[i].mouseChildren = false;
					updateBindingText(i);
				}
			}
			
			_backButton.addEventListener(MouseEvent.CLICK, backClickedHandler, false, 0, true);
			_backButton.buttonMode = true;
		}
		
		private function backClickedHandler(e:MouseEvent):void
		{
			_settings.flush();
			if (stage.hasEventListener(KeyboardEvent.KEY_DOWN))
				stage.removeEventListener(KeyboardEvent.KEY_DOWN, keyDownHandler);
				
			_settings = null;
			
			initGameMenu();
		}

		private function updateBindingText(selectedKey:int):void
		{
			var str:String = keyString(_settings.keyBindings[selectedKey]);
			if(selectedKey == _settings.mouseButton)
			{
				if( _settings.keyBindings[selectedKey] ) str += "/";
				str += "click";
			}
			_keyTexts[selectedKey].text.text = str;
		}
		
		private function changeBindingHandler(e:MouseEvent):void
		{
			for (var i:uint = 0; i < _keyTexts.length; i++)
				if (_keyTexts[i] == e.target)
					break;
			
			if (_selectedKey == i)
			{
				var oldButton:int = _settings.mouseButton;
				_settings.mouseButton = i;
				if( _settings.mouseButton >= 0 ) updateBindingText( oldButton );
				updateBindingText( i );
				stage.removeEventListener(KeyboardEvent.KEY_DOWN, keyDownHandler);
				return;
			}
			else if (_selectedKey != -1)
			{
				updateBindingText( _selectedKey );
			}

			stage.addEventListener(KeyboardEvent.KEY_DOWN, keyDownHandler, false, 0, true);
			_selectedKey = i;
			_keyTexts[i].text.text = "press a key";
		}
		
		private function keyDownHandler(e:KeyboardEvent):void
		{
			if (e.keyCode == Keyboard.ESCAPE) return;
			if (e.keyCode >= 0x30 && e.keyCode <= 0x39) return;
			
			_settings.keyBindings[_selectedKey] = e.keyCode;
			updateBindingText(_selectedKey);
			_selectedKey = -1;

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