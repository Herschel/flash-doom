package com.newgrounds
{
	import flash.display.Loader;
	import flash.events.Event;
	import flash.events.EventDispatcher;
	import flash.events.IOErrorEvent;
	import flash.events.SecurityErrorEvent;
	import flash.net.URLLoader;
	import flash.net.URLRequest;
	import flash.net.URLRequestMethod;
	import flash.net.URLVariables;
	
	public class NewgroundsAPICommand extends EventDispatcher
	{
		public static const
			CONNECT_MOVIE:uint				= 0,
			LOAD_CUSTOM_LINK:uint 			= 1,
			LOAD_FILE:uint					= 2,
			LOAD_FILE_LIST:uint				= 3,
			LOAD_MEDALS:uint				= 4,
			LOAD_MY_SITE:uint				= 5,
			LOAD_NEWGROUNDS:uint			= 6,
			LOAD_OFFICIAL_VERSION:uint		= 7,
			LOAD_SCORES:uint				= 8,
			LOG_CUSTOM_EVENT:uint			= 9,
			POST_SCORE:uint					= 10,
			SAVE_FILE:uint					= 11,
			UNLOCK_MEDAL:uint				= 12;
		
		private static const COMMAND_STRINGS:Array =
			[
				"connectMovie",
				"loadCustomLink",
				"loadFile",
				"loadFileList",
				"loadMedals",
				"loadMySite",
				"loadNewgrounds",
				"loadOfficialVersion",
				"loadScores",
				"logCustomEvent",
				"postScore",
				"saveFile",
				"unlockMedal"
			];
			
		private static var _loaders:Array = [];	// need to put URLLoaders here to prevent garbage collection
		
		private var _commandId:String;
		private var _movieId:uint;
		
		public function NewgroundsAPICommand(command:uint, movieId:uint)
		{
			_commandId = command;
			_movieId = movieId;
		}
		
		public function get command():String		{ return COMMAND_STRINGS[_commandId]; }
		public function get commandId():uint		{ return _commandId; }
		
		public function get movieId():uint			{ return _movieId; }
				
		public function sendTo(url:String):void
		{
			var urlRequest:URLRequest = new URLRequest(url + "?seed=" + Math.random());	// prevent caching
			urlRequest.data = createUrlVariables();
			urlRequest.method = URLRequestMethod.POST;

			var loader:URLLoader = new URLLoader(urlRequest);
			loader.addEventListener(Event.COMPLETE, eventHandler);
			loader.addEventListener(IOErrorEvent.IO_ERROR, eventHandler);
			loader.addEventListener(SecurityErrorEvent.SECURITY_ERROR, eventHandler);
			
			_loaders.push(loader);
		}
		
		public function toString():String
		{
			return "NewgroundsAPICommand: " + _command;
		}
		
		
		private function createUrlVariables():URLVariables
		{
			var urlVars:URLVariables = new URLVariables();
			urlVars.command = commandId;
						
			return urlVars;
		}
		
		private function eventHandler(e:Event):void
		{
			cleanupLoader( Loader(e.target) );
			dispatchEvent( e );
		}
		
		private function cleanupLoader(loader:Loader):void
		{
			loader.removeEventListener(Event.COMPLETE, eventHandler);
			loader.removeEventListener(IOErrorEvent.IO_ERROR, eventHandler);
			loader.removeEventListener(SecurityErrorEvent.SECURITY_ERROR, eventHandler);
			
			for (var i:uint = 0; i < _loaders.length; i++)
				if (_loaders[i] == loader)
				{
					_loaders.splice(i, 1);
					return;
				}
		}
	}
	
}