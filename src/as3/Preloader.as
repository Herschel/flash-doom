﻿/*-----------------------------------------------------------------------------
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
 *  ad-hoc preloader
 *  AUTHOR: Mike Welsh
 *-----------------------------------------------------------------------------
 */

package 
{
	import flash.display.MovieClip;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.net.navigateToURL;
	import flash.net.URLRequest;
	import flash.system.Capabilities;
	
	public class Preloader extends MovieClip 
	{
		public static const UPDATE_FLASH_URL:String = "http://get.adobe.com/flashplayer/";
		
		private var _urlRequest:URLRequest;
		
		// clips in DoomPreloader.swf
		public var _ngButton:Sprite;
		public var _updateFlashButton:Sprite;
		public var _playButton:Sprite;
		public var _loadBar:Sprite;
		
		private var _loaded:Boolean;
		
		public function Preloader()
		{
			stop();
			
			// check that we are at least flash 11 compatible
			if ( isFlashVersion(11,3) )
			{
				gotoAndStop( "loading" );
				
				_urlRequest = new URLRequest("http://www.newgrounds.com");
				
				_ngButton.buttonMode = true;
				_ngButton.useHandCursor = true;
				_ngButton.addEventListener( MouseEvent.CLICK, linkButtonClickHandler );
				
				addEventListener( Event.ENTER_FRAME, enterFrameHandler );
				enterFrameHandler( null );
				
				_loaded = false;
			}
			else
			{
				gotoAndStop( "flashUpdateNeeded" );
				
				_urlRequest = new URLRequest( UPDATE_FLASH_URL );
				
				_updateFlashButton.buttonMode = true;
				_updateFlashButton.useHandCursor = true;
				_updateFlashButton.addEventListener( MouseEvent.CLICK, linkButtonClickHandler );
			}
		}

		private function isFlashVersion(major:uint, minor:uint = 0, buildNumber:uint = 0, internalBuildNumber:uint = 0):Boolean
		{
			var version:Array = Capabilities.version.split(" ")[1].split(",");
			var requiredVersion:Array = arguments;
			
			for (var i:uint = 0; i < requiredVersion.length; i++)
				version[i] = uint(version[i]);

			for (i = 0; i < requiredVersion.length; i++)
			{
				if (version[i] > requiredVersion[i])
					return true;
				if (version[i] < requiredVersion[i])
					return false;	
			}
			
			return true;
		}
		
		private function enterFrameHandler(e:Event):void
		{
			if (!loaderInfo)
				return;
				
			if ( _loaded && _playButton && !_playButton.hasEventListener( MouseEvent.CLICK ) )
			{
				_playButton.addEventListener( MouseEvent.CLICK, playButtonClickHandler );
				_playButton.buttonMode = true;
				_playButton.useHandCursor = true;
			}
			
			if(!_loaded)
			{
				if( _loadBar )
					_loadBar.width = 3 * Math.floor( 100 * (loaderInfo.bytesLoaded / loaderInfo.bytesTotal) );
	
				if ( loaderInfo && loaderInfo.bytesLoaded == loaderInfo.bytesTotal )
				{
					gotoAndStop( "loadingComplete" );
					_loaded = true;
				}
			}
		}
		
		private function linkButtonClickHandler(e:MouseEvent):void
		{
			navigateToURL( _urlRequest, "_blank" );
		}
		
		private function playButtonClickHandler(e:MouseEvent):void
		{
			removeEventListener( Event.ENTER_FRAME, enterFrameHandler );
			_playButton.removeEventListener( MouseEvent.CLICK, playButtonClickHandler );
			gotoAndPlay("loadingTransition");
		}
		
		public function loadingTransitionComplete():void
		{
			_ngButton.removeEventListener( MouseEvent.CLICK, linkButtonClickHandler );
			dispatchEvent( new Event( Event.COMPLETE ) );
		}

	}
	
}