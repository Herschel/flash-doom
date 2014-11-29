package com.newgrounds
{
	import com.adobe.crypto.MD5;
	import com.adobe.serialization.json.JSON_;
	import flash.display.DisplayObject;
	import flash.display.DisplayObjectContainer;
	import flash.display.Loader;
	import flash.display.Shape;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.EventDispatcher;
	import flash.events.IOErrorEvent;
	import flash.net.navigateToURL;
	import flash.net.SharedObject;
	import flash.net.URLLoader;
	import flash.net.URLRequest;
	import flash.net.URLRequestMethod;
	import flash.net.URLVariables;
	import flash.system.Capabilities;
	import flash.system.Security;
	import flash.text.TextFormat;
	import flash.utils.Dictionary;
	import flash.utils.Timer;
	
	public class NewgroundsAPI extends EventDispatcher
	{				
		// set to true when debugging this class to see all the input/output traced to the output panel.
		private static var do_echo:Boolean = false;
		
		// HTTP Paths
		//private static var GATEWAY_URL = "http://ng-local.newgrounds.com/ngads/gateway_v2.php";
		//private static var AD_TERMS_URL = "http://ng-local.newgrounds.com/wiki/flashads/terms/";
		//private static var COMMANDS_WIKI_URL = "http://ng-local.newgrounds.com/wiki/flashapi/commands/";
		// HTTP Paths
		private static const GATEWAY_URL:String = "http://www.ngads.com/gateway_v2.php";
		private static const AD_TERMS_URL:String = "http://www.newgrounds.com/wiki/flashads/terms/";
		private static const COMMANDS_WIKI_URL:String = "http://www.newgrounds.com/wiki/flashapi/commands/";
		
		private static var _initialized:Boolean = false;
		
		// class variables
		private static var tracker_id:uint, movie_id:String, host:String, encryption_key:String, connected:Boolean, debug:Boolean, version:String, ad_url:String, ad_swf_url:String;
		private static var ad_reset:Number = 0;
		private static var save_file:* = null; // NewgroundsAPISaveFile
		private static var publisher_id:Number, session_id:String, user_email:String, user_name:String, user_id:Number;
		private static var medals:Array = null;
		private static var timeoutTimer:Timer;
		
		private static var root:DisplayObject;
		private static var loaders:Array = [];
		private static var flashAdTarget:DisplayObjectContainer;
		private static var adContainer:Sprite;
		private static var ad:Loader;
		private static var adURLLoader:URLLoader;
		
		// removed from stage was only added in 9.0.28. Avoid an error for people using earlier version.
		private static const REMOVED_FROM_STAGE:String = "removedFromStage";
		
		// score vars
		private static var score_page_counts:Object = new Object();
		// encoder vars
		private static var compression_radix:String = "/g8236klvBQ#&|;Zb*7CEA59%s`Oue1wziFp$rDVY@TKxUPWytSaGHJ>dmoMR^<0~4qNLhc(I+fjn)X";
		private static var compressor:BaseN = new BaseN(compression_radix);
		
		public static var errors:Dictionary = NewgroundsAPIError.init_codes();
		
		private static var sharedObjects:Object = new Object();
		
		public static function initialize(root:DisplayObject):void
		{
			if (_initialized) return;
			_initialized = true;
			
			NewgroundsAPI.root = root;
			timeoutTimer = new Timer(8000, 1);
			
			var flashVars:Object;
			if (root.loaderInfo)
			{
				flashVars = root.loaderInfo.parameters;
				host = root.loaderInfo.url;
			}
		
			if (flashVars)
			{
				// see if a username was provided.
					user_name = flashVars.NewgroundsAPI_UserName;
				// and a user id
					user_id = flashVars.NewgroundsAPI_UserID;
					publisher_id = flashVars.NewgroundsAPI_PublisherID;
					session_id = flashVars.NewgroundsAPI_SessionID;
			} else {
				publisher_id = 1;
				session_id = null;
				user_id = 0;
				user_name = "Guest";
			}
			
			if (host.indexOf("http://") > -1 || host.indexOf("https://") > -1) {
				host = host.split("/")[2].toLowerCase();	// TODO
			} else {
				host = 'localhost';
			}
		}
		
		//=================================== API Configuration ==================================\\
		
		// set the movie version for version control
		public static function setMovieVersion(v:String):void
		{
			if (v) {
				version = String(v);
			}
		}
		
		// set the user's email address (only used if there is no session_id)
		public static function setUserEmail(e:String):void
		{
			user_email = e;
		}
		
		//========================================== Lookup Functions ==================================\\
		
		// pass the URL that will track a referral and redirect to the official version of this movie
		public static function getOfficialVersionURL():String
		{		
			var o_url:String = GATEWAY_URL+"?tracker_id="+movie_id+"&command_id="+getCommandID('loadOfficalVersion')+"&seed="+Math.random();
			
			if (debug) {
				o_url += "&debug=1";
			}
			return o_url;
		}
		
		// check to see if the hosting site has provided a user session
		public static function hasUserSession():Boolean
		{
			return session_id != null && session_id != "" && publisher_id != 0;
		}
		
		public static function isNewgrounds():Boolean
		{
			return (publisher_id == 1 || getHost().toLowerCase().indexOf("ungrounded.net") > -1);
		}
		
		public static function hasPublisher():Boolean
		{
			return publisher_id != 0;
		}
		
		// check to see if the user has provided an email address
		public static function hasUserEmail():Boolean
		{
			return user_email != null && user_email != "";
		}
		
		//=================================== Gateway Commands ===================================\\
		
		public static function connectionTimeOut(e:Event = null):void
		{
			callListener(events.MOVIE_CONNECTED, false, new NewgroundsAPIError("CONNECTION_FAILED","Connection to NewgroundsAPI gateway timed out."));
		}
		
		// this is a quiasi constructor that gets base data about the API entry and loads settings from the API Gateway
		public static function connectMovie(root:DisplayObject, m_id:String, encrypt_key:String, debug_mode:Boolean):void
		{
			initialize(root);
			
			// just skip everything if this has already been called
			if (connected) { return; }
			
			timeoutTimer.start();
			
			// if movie_id wasn't provided, the API just can't be used... period
			if (!m_id) {
				fatalError("NewgroundsAPI.connectMovie() - missing required movie_id parameter", 'connectMovie'); 
			}
			
			// make sure the movie id is a string		
			movie_id = String(m_id);
			
			// get the numeric id used to track this movie
			tracker_id = uint(movie_id.substring(0, movie_id.indexOf(":")));
			
			// set the other parameter vars...
			encryption_key = encrypt_key;
			debug = debug_mode;
				
			// the 'connection' is set to true so we know we have the tracket_id and other pertinant information
			connected = true;
			
			sendCommand('connectMovie', {host:getHost(), movie_version:version});
		}
		
		// figure out what domain the swf is being hosted from		
		private static function getHost():String
		{// these 2 values are used for read-only lookups, so if they get hacked they can't be used to cheat
			return host;
		}
		// SITE REFERRALS \\
		
		// loads Newgrounds in a new window and tracks the referral
		public static function loadNewgrounds():void
		{
			sendCommand('loadNewgrounds', {host:getHost()}, true);
		}
		
		// loads the author's primary site in a new window and tracks the referral
		public static function loadMySite():void
		{
			sendCommand('loadMySite', {host:getHost()});
		}
		
		// loads the url associated with the link name in a new window and tracks the referral
		public static function loadCustomLink(link:String):void
		{
			sendCommand('loadCustomLink', {host:getHost(), link:link}, true);
		}
		
		// CUSTOM EVENTS \\
		
		// tracks the custom event
		public static function logCustomEvent(event:String):void
		{
			sendCommand('logCustomEvent', {host:getHost(), event:event});
		}
		
		// HIGH SCORES \\
		
		// posts a high score
		public static function postScore(score:String, value:Number, get_best:Boolean):void
		{			
			sendSecureCommand('postScore', {score:score, value:value, get_best:get_best});
		}
		
		public static function getTodaysScores(score:String, params:Object):void
		{
			getScores(score, "t", params, 'getTodaysScores');
		}
	
		public static function getYesterdaysScores(score:String, params:Object):void
		{
			getScores(score, "y", params, 'getYesterdaysScores');
		}
	
		public static function getThisWeeksScores(score:String, params:Object):void
		{
			getScores(score, "w", params, 'getThisWeeksScores');
		}
		
		public static function getThisMonthsScores(score:String, params:Object):void
		{
			getScores(score, "m", params, 'getThisMonthsScores');
		}
		
		public static function getThisYearsScores(score:String, params:Object):void
		{
			getScores(score, "y", params, 'getThisYearsScores');
		}
		
		public static function getAlltimeScores(score:String, params:Object):void
		{
			getScores(score, "a", params, 'getAlltimeScores');
		}
		
		// valid params are 'user_id'
		public static function getScores(score:String, period:String, params:Object = null, command_name:String = null):void
		{			
			if (!params) {
				params = new Object();
			}
			
			if (!hasUserSession()) {
				callListener(events.SCORES_LOADED, false, new NewgroundsAPIError("SITE_ID_REQUIRED", "Host '"+getHost()+"' does not have high scores enabled"));
				return;
			}
			
			params.publisher_id = publisher_id;
			params.period = period;
			params.score = score;
			
			var period_tag:String;
			if (params.user_id) {
				period_tag = period;
			} else {
				period_tag = period+"-u";			
			}
			
			if (score_page_counts[period_tag] == undefined) {
				params.request_page_count = true;
			}
			sendCommand('getScores', params);
		}
		
		// MEDALS (Achievements) \\
		
		public static function unlockMedal(medal:String, get_score:Boolean = false):void
		{
			if (!medal) {
				sendError( {command_id:getCommandID('unlockMedal')}, new NewgroundsAPIError("MISSING_PARAM", "missing required medal name") );
				return;
			}
			
			var params:Object = new Object();
			params.medal = medal; 
			params.get_score = get_score;
			sendSecureCommand('unlockMedal', params);
		}
		
		public static function loadMedals():void
		{
			if (medals) {
				callListener(events.MEDALS_LOADED, true, {medals:medals});
				return;
			}
			
			var params:Object = new Object();
			if (hasUserSession()) {
				params.publisher_id = publisher_id;
				params.user_id = user_id;
			}
			sendCommand('getMedals',params);
		}
		
		public static function getMedals():Array
		{
			return medals;
		}
		
		// LOCAL SAVES \\
		public static function saveLocal(save_id:String, save_data:*, size_allocation:uint = 0):void // size allocation?
		{
			var sharedObj:SharedObject;
			if (!sharedObjects[save_id]) {
				sharedObjects[save_id] = SharedObject.getLocal("ng_ap_secure_"+movie_id+"_"+save_id);
			}
			
			sharedObj = sharedObjects[save_id];
			
			sharedObj.data[save_id] = encodeData(save_data);
			
			sharedObj.flush();
		}
		
		public static function loadLocal(save_id:String):*
		{
			var sharedObj:SharedObject;
			
			if (!sharedObjects[save_id]) {
				sharedObjects[save_id] = SharedObject.getLocal("ng_ap_secure_"+movie_id+"_"+save_id);
			}
			sharedObj = sharedObjects[save_id];
			
			sharedObj.flush();
			
			if (sharedObj.data[save_id]) {
				return decodeData(sharedObjects.data[save_id]);
			} else {
				return null;
			}
		}
		
		public static function encodeData(data:Object):String
		{
			return compressHex(RC4.encrypt(JSON_.encode(data), encryption_key));
		}
		
		public static function decodeData(base:String):*
		{
			
			return JSON_.decode(RC4.decrypt(uncompressHex(base), encryption_key));
		}
		
		private static function compressHex(hex_value:String):String
		{
			// our data will ultimately be converted by reading 6-character chunks of hex code and compressing it to 4-char baseN code
			// Because it's unlikely that we'll have an even 6 characters at the end of this code, we need to take a not of what's 
			// really going to be left over.
			var offset:uint = hex_value.length % 6;
			
			// now we can read through our hex string and convert each 6-char chunk to a 4-char baseN format
			var basen_value:String = "";
			for(var i:uint=0; i<hex_value.length; i+=6) {
				basen_value += compressor.encode( uint("0x" + hex_value.substr(i, 6)), 4);
			}
			
			// and now we stick our compressed data to our offset so PHP has all the info it needs
			return offset.toString() + basen_value;
		}
		
		private static function uncompressHex(base_value:String):String
		{
			var offset:uint = uint( base_value.charAt(0) );
			var hex_value:String = "";
			var hl:uint;
			
			for(var i:uint=1; i<base_value.length; i+=4) {
				var chunk:String = base_value.substr(i,4);
				var num:uint = uint( compressor.decode(chunk) );
				var hex:String = num.toString(16);
				
				if (i+4 < base_value.length) {
					hl = 6;
				} else {
					hl = offset;
				}
				while (hex.length < hl) {
					hex = "0"+hex;
				}
				hex_value += hex;
			}
			
			return hex_value;
		}
		
		// FILE SAVES \\
		
		// saving files is actually a multi-step process mainly contained within the NewgroundsAPISaveFile class.
		// This function starts the process
		public static function saveFile(folder:String, filename:String, contents:Object, thumbnail_source:*):void
		{
			/*// to protect against being hammered by large packets, we only allow for one upload at a time
			if (!save_file) {
				// create a file instance
				save_file = new NewgroundsAPISaveFile(movie_id, folder, filename);
				save_file.setContents(contents);
				if (thumbnail_source) {
					save_file.setThumbnail(thumbnail_source);
				}
				
				save_file.onError = function(msg:String)
				{
					var error:NewgroundsAPIError = new NewgroundsAPIError('PERMISSION_DENIED', msg);
					sendError({command_id:getCommandID('saveFile')}, error);
					callListener(events.FILE_SAVED, false, error);
				}
				
				save_file.onCancel = function(msg)
				{
					var error:NewgroundsAPIError = new NewgroundsAPIError('USER_CANCELLED', msg);
					callListener(events.FILE_SAVED, false, error);
				}
				
				save_file.submit();
				
			// if another upload is in transit, we'll pass back an error.
			} else {
				var error:NewgroundsAPIError = new NewgroundsAPIError("UPLOAD_IN_PROGRESS", "Please wait for the previous file to finish uploading");
				sendError({command_id:getCommandID('saveFile')}, error);
				callListener(events.FILE_SAVED, false, error);
			}*/
			// MIKE todo
		}
		
		// This function checks a folder/file pair to see if the calling user has sufficient permissions to write to it.
		// This function can be used standalone, but is also used by the save_file instance. 
		public static function checkFilePrivs(folder:String, filename:String):void
		{
			var user:uint
			if (user_id) {
				user = user_id;
			} else {
				user = 0;
			}
			
			var params:Object = {folder:folder, filename:filename, user_id:user, publisher_id:publisher_id};
			sendCommand('checkFilePrivs', params);
		}
		
		// When the save_file instance has gathered permssions and such, this command posts the actual file and thumbnail data
		public static function finishFileSave(folder:String, filename:String, description:String, share:Boolean, file:*, thumbnail:*):void // TODO
		{
			/*// params that will get encrypted
			var params:Object;
			params = {
				folder:folder,
				filename:filename,
				description:description,
				share:share
			};
			
			// encrypting large file and image data would probably crash flash, so we don't bother with that
			var files:Object;
			files = {
				file:file,
				thumbnail:thumbnail
			};
			
			// This is the last step in the process and the command name is passed as 'saveFile' since it's the command that starts all of this stuff
			sendSecureCommand('saveFile', params, null, files);*/
		}
		
		public static function getFiles(folder:String, options:Object):void // TODO
		{
			/*
			// keys that can be used to sort results on
			var sort_options:Object = {
				name:1,
				date:2,
				score:3
			};
			
			// defaults for each option
			var default_options:Object = {
				user_only:false,
				sort_on:'date',
				page:1,
				results_per_page:20,
				sort_descending:true
			};
			
			var valid_keys = new Array();
			for(var i in sort_options) {
				valid_keys.push("'"+i+"'");
			}
	
			var valid_options:Array = new Array();
			for(var o in default_options) {
				valid_options.push("'"+o+"'");
			}
			
			// check the sort_on option for a bad key
			if (options.sort_descending && !sort_options[options.sort_descending]) {			
				var error = new NewgroundsAPIError("MISSING_PARAM", "'"+options.sort_descending+"' is not a valid sort_on value.  Valid values are: " + valid_keys.join(", "));
				sendError({command_id:getCommandID('getFiles')}, error);
				delete options.sort_descending;
			}
			
			// declare the params object...
			var params:Object;
			
			// take a look at any other options and validate them
			if (options) {
				for(var i in options) {
					
					// If there isn't a default available, it's not a valid option
					if (default_options[i] == undefined) {
						
						var error = new NewgroundsAPIError("MISSING_PARAM", "'"+i+"' is not a valid option.  Valid options are: " + valid_options.join(", "));
						sendError({command_id:getCommandID('getFiles')}, error);
						
						delete options[i];
						
					// if the datatype isn't the same as the default option's, it isn't valid
					} else if (typeof(options[i]) != typeof(default_options[i])) {
						
						var error = new NewgroundsAPIError("MISSING_PARAM", "option '"+i+"' should be the following type: "+typeof(default_options[i]));
						sendError({command_id:getCommandID('getFiles')}, error);
						
						delete options[i];					
					}
				}
				
				// if we have options, we'll copy them to the params object
				params = options;
			} else {
				
				// if we have no options we need to make a blank object for params
				params = new Object();
			}
			
			if (hasUserSession()) {
				params.publisher_id = publisher_id;
				params.user_id = user_id;
			}
			
			params.folder = folder;
			
			sendCommand('getFiles', params);
			*/
		}
		
		//============================================ BUILT-IN EVENT RESPONSES ========================================\\
		
		// if something has an error but can proceed with user confirmation, this event will pass the error along with confirm and cancel functions
		public static function getConfirmation(command_name:String, event:uint, msg:String, target:Object, confirm:String, cancel:String):void {
			// TODO: ???
			/*var error:NewgroundsAPIError = new NewgroundsAPIError("CONFIRM_REQUEST", msg);
			error.confirm = function() {
					target[confirm]();
			}
			error.cancel = function() {
					target[cancel]();
			}
			error.command = getCommandID(command_name);
			callListener(event, false, error);*/
		}
		
		// if the author doesn't have a custom handler for blocking movies:
		private static function doBlockHost(event:Object):void
		{
			/*
			// stop the movie
			root.stop();
			
			// load our text formats
			initTextFormats();
			
			// draw a full-screen overlay
			_root.createEmptyMovieClip('NGAPI_deny_host_overlay',_root.getNextHighestDepth());
			_root.NGAPI_deny_host_overlay.lineStyle(20,0x000000,100);
			_root.NGAPI_deny_host_overlay.beginFill(0x660000);
			_root.NGAPI_deny_host_overlay.moveTo(0,0);
			_root.NGAPI_deny_host_overlay.lineTo(Stage.width,0);
			_root.NGAPI_deny_host_overlay.lineTo(Stage.width,Stage.height);
			_root.NGAPI_deny_host_overlay.lineTo(0,Stage.height);
			_root.NGAPI_deny_host_overlay.lineTo(0,0);
			_root.NGAPI_deny_host_overlay.endFill();
			
			// build the on-screen message
			var message = "This movie has not been approved for use on "+getHost()+".";
			message += newline+newline+"For an aproved copy, please visit:"+newline;
			
			// make a note of where the link text of the legal copy will start
			var link_start = message.length;
			
			// append the link html
			message += event.data.movie_url;
			
			// marke the end of the link text
			var link_end = message.length;
			
			// draw a fullscreen text field that blocks the mouse from being able to hit buttons
			_root.NGAPI_deny_host_overlay.createTextField('mousekill',100,0,0,Stage.width,Stage.height);
	
			// draw the word ERROR
			_root.NGAPI_deny_host_overlay.createTextField('error',101,(Stage.width-400)/2,(Stage.height/2)-100,400,200);
			_root.NGAPI_deny_host_overlay.error.text = 'ERROR!';
			_root.NGAPI_deny_host_overlay.error.setTextFormat(error_format);
			
			// draw the error message
			_root.NGAPI_deny_host_overlay.createTextField('message',102,(Stage.width-400)/2,Stage.height/2,400,200);
			_root.NGAPI_deny_host_overlay.message.text = message;
			_root.NGAPI_deny_host_overlay.message.multiline = true;
			_root.NGAPI_deny_host_overlay.message.wordWrap = true;
			_root.NGAPI_deny_host_overlay.message.html = true;
			_root.NGAPI_deny_host_overlay.message.setTextFormat(normal_format);
			
			// make and style the hyperlink
			link_format.url = event.data.redirect_url;
			_root.NGAPI_deny_host_overlay.message.setTextFormat(link_start,link_end,link_format); TODO
			*/
		}
		
		public static function onNewVersionAvailable(event:Object, params:Object):void
		{
			/* TODO this!
			// stop the movie
			_root.stop();
			
			// load our text formats
			initTextFormats();
			
			// get the center of the movie
			var center = new Object();
			center.x = Stage.width/2;
			center.y = Stage.height/2;
	
			// draw a translucent fullscreen box
			_root.createEmptyMovieClip('NGAPI_new_version_overlay',_root.getNextHighestDepth());
			_root.NGAPI_new_version_overlay.lineStyle(1,0x000000,100);
			_root.NGAPI_new_version_overlay.beginFill(0x000000,70);
			_root.NGAPI_new_version_overlay.moveTo(-10,-10);
			_root.NGAPI_new_version_overlay.lineTo(-10,1000);
			_root.NGAPI_new_version_overlay.lineTo(1000,1000);
			_root.NGAPI_new_version_overlay.lineTo(1000,-10);
			_root.NGAPI_new_version_overlay.lineTo(-10,-10);
			_root.NGAPI_new_version_overlay.endFill();
			
			// draw the blue popup box
			_root.NGAPI_new_version_overlay.lineStyle(10,0x000000,100);
			_root.NGAPI_new_version_overlay.beginFill(0x000033);
			_root.NGAPI_new_version_overlay.moveTo(center.x-240,center.y-120);
			_root.NGAPI_new_version_overlay.lineTo(center.x+240,center.y-120);
			_root.NGAPI_new_version_overlay.lineTo(center.x+240,center.y+80);
			_root.NGAPI_new_version_overlay.lineTo(center.x-240,center.y+80);
			_root.NGAPI_new_version_overlay.lineTo(center.x-240,center.y-120);
			_root.NGAPI_new_version_overlay.endFill();
	
			// draw the [X] box to close the popup
			_root.NGAPI_new_version_overlay.createEmptyMovieClip('exit',1000);
			_root.NGAPI_new_version_overlay.exit.lineStyle(2,0x0099ff,100);
			_root.NGAPI_new_version_overlay.exit.beginFill(0x000000,50);
			_root.NGAPI_new_version_overlay.exit.moveTo(center.x+210,center.y-110);
			_root.NGAPI_new_version_overlay.exit.lineTo(center.x+230,center.y-110);
			_root.NGAPI_new_version_overlay.exit.lineTo(center.x+230,center.y-90);
			_root.NGAPI_new_version_overlay.exit.lineTo(center.x+210,center.y-90);
			_root.NGAPI_new_version_overlay.exit.lineTo(center.x+210,center.y-110);
			_root.NGAPI_new_version_overlay.exit.endFill();
			
			// draw the X in the [X] box
			_root.NGAPI_new_version_overlay.exit.moveTo(center.x+214,center.y-106);
			_root.NGAPI_new_version_overlay.exit.lineTo(center.x+226,center.y-94);
			_root.NGAPI_new_version_overlay.exit.moveTo(center.x+226,center.y-106);
			_root.NGAPI_new_version_overlay.exit.lineTo(center.x+214,center.y-94);
			
			// code that makes the [X] box close the popup
			_root.NGAPI_new_version_overlay.exit.onMouseUp = function() {
				if (_root.NGAPI_new_version_overlay.exit.hitTest(_root._xmouse,_root._ymouse)) {
					_root.NGAPI_new_version_overlay.removeMovieClip();
				}
			}
			
			// start the version message
			var message = "Version "+event.data.movie_version+" is now available at:"+newline;
			
			// take a note of where the hyperlink starts
			var link_start = message.length;
			
			// append the link
			message += event.data.movie_url;
			
			// note where the link ends
			var link_end = message.length;
			
			// make a fullscreen textarea to prevent the mouse from working on items behind the popup
			_root.NGAPI_new_version_overlay.createTextField('mouseblocker',99,-10,-10,1000,1000);
			
			// write the 'new version available' header
			_root.NGAPI_new_version_overlay.createTextField('newversion',100,center.x-210,center.y-90,400,80);
			_root.NGAPI_new_version_overlay.newversion.text = 'New Version Available!';
			_root.NGAPI_new_version_overlay.newversion.setTextFormat(header_format);
			
			// write the new version message body
			_root.NGAPI_new_version_overlay.createTextField('message',101,(Stage.width-400)/2,Stage.height/2,400,40);
			_root.NGAPI_new_version_overlay.message.text = message;
			_root.NGAPI_new_version_overlay.message.multiline = true;
			_root.NGAPI_new_version_overlay.message.wordWrap = true;
			_root.NGAPI_new_version_overlay.message.html = true;
			_root.NGAPI_new_version_overlay.message.setTextFormat(normal_format);
			
			// make and style the hyperlink to the new version
			link_format.url = event.data.redirect_url;
			_root.NGAPI_new_version_overlay.message.setTextFormat(link_start,link_end,link_format);*/
		}
		
		// text style declarations
		private static var error_format:TextFormat;
		private static var header_format:TextFormat;
		private static var normal_format:TextFormat;
		private static var link_format:TextFormat;
		
		private static function initTextFormats():void {
			if (!error_format) {
				error_format = new TextFormat();
				error_format.font = "Arial Black";
				error_format.size = 48;
				error_format.color = 0xFF0000;
			}
			if (!header_format) {
				header_format = new TextFormat();
				header_format.font = "Arial Black";
				header_format.size = 24;
				header_format.color = 0xFFFFFF;
			}
			if (!normal_format) {
				normal_format = new TextFormat();
				normal_format.font = "Arial";
				normal_format.bold = true;
				normal_format.size = 12;
				normal_format.color = 0xFFFFFF;
			}
			if (!link_format) {			
				link_format = new TextFormat();
				link_format.color = 0xFFFF00;
				link_format.underline = true;
			}
		}
		
		//============================================ EVENT HANDLING =====================================\\
		
		// handle response packets from the API Gateway
		private static function doEvent(e:Object):void
		{
			var msg:String;
			var packet:Object;
			var user:String;
			
			switch (getCommandName(e.command_id)) {
				
				// the primary response is when you run connectMovie.
				// This response handles movie protection, version control AND flash ad permissions.
				case "connectMovie":
				
					timeoutTimer.stop();
					
					// handle base connection
					sendMessage("You have successfully connected to the Newgrounds API Gateway");
					sendMessage("Movie identified as \""+e.movie_name+"\"");
					callListener(events.MOVIE_CONNECTED, e.success, {movie_name:e.movie_name});
					
					// FLASH ADS \\
					
					var fake_ad:Boolean = false;
					
					// handle responses on movies that were not approved
					if (e.ad_status === -1) {
						msg = "This movie was not approved to run Flash Ads.";
						sendWarning(msg);
						sendWarning("visit "+AD_TERMS_URL+" to view our approval guidelines");
						if (!e.ad_url) {
							callListener(events.ADS_APPROVED, false, new NewgroundsAPIError("FLASH_ADS_NOT_APPROVED",msg));
						} else {
							fake_ad = true;
						}
						
					// handle ads on movies still awaiting approval from NG
					} else if (e.ad_status === 0) {
						msg = "Flash Ads are currently awaiting approval.";
						sendNotice(msg);
						if (!e.ad_url) {
							callListener(events.ADS_APPROVED, false, new NewgroundsAPIError("FLASH_ADS_NOT_APPROVED",msg));
						} else {
							fake_ad = true;
						}
					}
					
					// handle approved flash ads
					if (e.ad_url) {
						ad_url = unescape(e.ad_url);
						if (!fake_ad) {
							sendMessage("This movie has been approved to run Flash Ads!");
						}
						callListener(events.ADS_APPROVED, true);
					} 
					
					// MOVIE PROTECTION \\
					
					if (e.deny_host) {
						msg = getHost()+" does not have permission to run this movie!";
						sendWarning(msg);
						sendWarning("	Update your API configuration to unblock "+getHost());
						callListener(events.HOST_BLOCKED, true, {movie_url:unescape(e.movie_url), redirect_url:getOfficialVersionURL()});
					}
					
					// VERSION CONTROL \\
					
					if (e.movie_version) {
						sendWarning("According to your API Configuration, this version is out of date.");
						if (version) {
							sendWarning("	The this movie is version "+version);
						}
						sendWarning("	The most current version is "+e.movie_version);
	
						callListener(events.NEW_VERSION_AVAILABLE, true, {movie_version:e.movie_version, movie_url:unescape(e.movie_url), redirect_url:getOfficialVersionURL()});
					}
					
					// PORTAL SUBMISSION DETECTION \\
					
					if (e.request_portal_url) {
						sendCommand('setPortalID', {portal_url:host}); // TODO: host could be wrong??
					}
					
					break;
					
				// CUSTOM EVENTS \\
				
				case "logCustomEvent":
					if (e.success) {
						sendMessage("Event '"+e.event+"' was logged.");
					}
					callListener(events.EVENT_LOGGED, e.success, {event:e.event});
					break;
					
				// HIGH SCORES \\
				
				case "postScore":
										
					if (e.success) {
						user = "User";
						
						if (user_email) {
							user = user_email;
						} else if (user_name) {
							user = user_name;
						}
						
						sendMessage(user+" posted "+e.value+" to '"+e.score+"'");
						packet = {score:e.score, value:e.value, username:user};
					}
					
					callListener(events.SCORE_POSTED, e.success, packet);
					break;
					
				case "getScores":
					var period_tag:String;
					
					if (e.user_id) {
						period_tag = e.period;
					} else {
						period_tag = e.period+"-u";			
					}
					
					if (e.total_pages) {
						score_page_counts[period_tag] = e.total_pages;
					}
						
					packet.user_id = e.user_id;
					packet.current_page = e.current_page;
					packet.total_pages = score_page_counts[period_tag];
					packet.scores = e.scores;
					packet.period = getPeriodName(e.period);
					
					callListener(events.SCORES_LOADED, e.success, packet);
					break;
					
				case "unlockMedal":
									
					if (medals) {
						for(var i:uint=0; i<medals.length; i++) {
							if (medals[i].medal_name === e.medal_name) {
								medals[i].medal_unlocked = true;
								break;
							}
						}
					}
					
					packet = {medal_name:e.medal_name, medal_value:e.medal_value, medal_difficulty:e.medal_difficulty};
					
					callListener(events.MEDAL_UNLOCKED, e.success, packet);
					break;
				
				case "getMedals":
				
					medals = e.medals;
										
					packet = {medals:e.medals}
					
					callListener(events.MEDALS_LOADED, e.success, packet);
					
					break;
				
				// SAVE FILES \\
				
				case "getFiles":
				
					break;
				
				case "getSystemFiles":
				
					break;
					
				case "saveFile":
				
					// clear the save_file instance so new files can be saved later
					save_file = null;
					
					packet = {
						file_id:e.file_id,
						filename:e.filename,
						file_url:e.file_url,
						thumbnail:e.thumbnail,
						icon:e.icon
					}
					
					callListener(events.FILE_SAVED, e.success, packet);
					break;
				
				case "checkFilePrivs":
					if (save_file) {
						save_file.checkPrivs(e);
					} else {
						packet = {
							filename:e.filename,
							folder:e.folder,
							can_read:e.can_read,
							can_write:e.can_write
						};
						
						callListener(events.FILE_PRIVS_LOADED, e.success, packet);
					}
					break;
			}
		}
		
		// define Event IDs
		public static const events:Object = {
				MOVIE_CONNECTED:		1,
				ADS_APPROVED:			2,
				AD_ATTACHED:			3,
				HOST_BLOCKED:			4,
				NEW_VERSION_AVAILABLE:	5,
				EVENT_LOGGED:			6,
				SCORE_POSTED:			7,
				SCORES_LOADED:			8,
				MEDAL_UNLOCKED:			9,
				MEDALS_LOADED:			10,
				FILE_PRIVS_LOADED:		11,
				FILE_SAVED:				12
		};
	
		// create array for event listeners
		private static var listeners:Array = setDefaultListeners();
		
		// build default event listeners for built-in responses
		private static function setDefaultListeners():Array
		{
			var defaults:Array = new Array();
			defaults[events.HOST_BLOCKED] = {listener:doBlockHost};
			defaults[events.NEW_VERSION_AVAILABLE]= {listener:onNewVersionAvailable};
			
			return defaults;
		}
		
		// function for adding evet listeners
		public static function addEventListener(event:uint, listener:Function, params:Object = null):void
		{
			listeners[event] = {listener:listener, params:params};
		}
		
		// function for clearing event listeners
		public static function removeEventListener(event:uint):void
		{
			delete(listeners[event]);
		}	
		
		// function for grabbing event names
		public static function getEventName(event:uint):String
		{
			for(var i:String in events) {
				if (events[i] == event) {
					return i;
				}
			}
			return undefined;
		}

		// this function passes data or error objects to appropriate event listeners
		private static function callListener(event:uint, success:Boolean, data:*=undefined, target:*=undefined):void
		{
			echo("Fired Event: "+getEventName(event));		
	
			// check if we have a listener assigned to the calling event
			if (listeners[event]) {
				// check to see if the result is an error and pass it in the appropriate object format
				if (data is NewgroundsAPIError) {
					listeners[event].listener({event:event,success:success,error:data,target:target},listeners[event].params);			
				} else {
					listeners[event].listener({event:event,success:success,data:data,target:target},listeners[event].params);
				}
			}
		}
		
		//===================================== Command Indexing =========================================\\
		
		private static function getCommandName(id:String):String
		{
			return(id);
		}
		
		private static function getCommandID(name:String):String
		{
			return(name);
		}
		
		//======================================= Time Aliases & Names ==================================\\
		
		public static var periods:Object = getPeriodAliases();
		
		private static var period_aliases:Object = {
			t:{name:"Today", alias:"TODAY"},
			p:{name:"Yesterday", alias:"YESTERDAY"},
			w:{name:"This Week", alias:"THIS_WEEK"},
			m:{name:"This Month", alias:"THIS_MONTH"},
			y:{name:"This Year", alias:"THIS_YEAR"},
			a:{name:"All-Time", alias:"ALL_TIME"}
		};
		
		private static function getPeriodAliases():Object
		{
			var aliases:Object = new Object();
			for(var i:String in period_aliases) {
				aliases[period_aliases[i].alias] = i;
			}
			return aliases;
		}
		
		public static function getPeriodName(p:String):String
		{
			for(var i:String in period_aliases) {
				if (i == p) {
					return period_aliases[i].name;
				}
			}
			
			return null;
		}
		
		public static function getPeriodAlias(p:String):String
		{
			for(var i:String in period_aliases) {
				if (i == p) {
					return period_aliases[i].alias;
				}
			}
			
			return null;
		}
		
		//====================================== Error Handling ==========================================\\
		
		// if the gateway responds with an error, this function dumps the error so the author can debug their work.
		private static function sendError(c:Object, e:NewgroundsAPIError):void
		{
			trace("[NewgroundsAPI ERROR] :: "+getCommandName(c.command_id)+"() - "+e.name+":\n				"+e.message);
		}
		
		// if the gateway responds with an error, this function dumps the error so the author can debug their work.
		private static function sendWarning(m:String,c:String = null):void
		{
			if (c) {
				m += "\n[NewgroundsAPI WARNING] :: 	See "+COMMANDS_WIKI_URL+c.toLowerCase()+" for additional information.";
			}
			
			trace("[NewgroundsAPI WARNING] :: "+m);
		}
		
		// if the gateway responds with an error, this function dumps the error so the author can debug their work.
		private static function sendNotice(m:String,c:String = null):void
		{
			if (c) {
				m += "\n[NewgroundsAPI NOTICE] :: 	See "+COMMANDS_WIKI_URL+c.toLowerCase()+" for additional information.";
			}
			
			trace("[NewgroundsAPI NOTICE] :: "+m);
		}
		
		// if this class is used incorrectly, this function will inform the author.
		private static function fatalError(m:String,c:String):void
		{
			if (c) {
				m += "\n	See "+COMMANDS_WIKI_URL+c.toLowerCase()+" for additional information.";
			}
			// throw the error and kill further script execution
			
			throw new Error("***ERROR*** class=NewgroundsAPI\n\n"+m);
		}
		
		//============================================= Gateway Comminication ======================================================\\
		
		public static function sendSecureCommand(command:String, secure_params:Object, unsecure_params:Object=null, files:Object=null):void
		{
			if (!debug && !hasUserSession() && !hasUserEmail()) {
				sendError({command_id:getCommandID(command)}, new NewgroundsAPIError("IDENTIFICATION_REQUIRED", "You must be logged in or provide an e-mail address ( using NewgroundsAPI.setUserEmail(\"name@domain.com\"); ) to use "+command+"()."));
				return;
			}
			
			if (!command) {
				fatalError("Missing command", "sendSecureCommand");
			}
			if (!secure_params) {
				fatalError("Missing secure_params", "sendSecureCommand");
			}
			
			if (!unsecure_params) {
				unsecure_params = new Object();
			}
			
			// make a random seed for validating the encryption
			var seed:String = "";
			for (var i:uint = 0; i < 16; i++)
			{
				seed += compression_radix.charAt( Math.floor(Math.random()*compression_radix.length) );
			}
			
			// add required data to the secure params
			if (debug) {
				secure_params.session_id = "";
			} else {
				secure_params.session_id = session_id;
			}
			secure_params.as_version = 3;
			secure_params.user_email = user_email;
			secure_params.publisher_id = publisher_id;
			secure_params.seed = seed;
			secure_params.command_id = getCommandID(command);
			
			// get the md5 value of our seed.  This is a hex format
			var hash:String = MD5.hash(seed);
			// encode and encrypt our secure params
			var rc4enc:String = RC4.encrypt(JSON_.encode(secure_params), encryption_key);
			// Merge the resulting hex string with the md5 hash
			var hex_value:String = hash+rc4enc;
			
			// and now we stick our compressed data to our offset so PHP has all the info it needs
			unsecure_params.secure = compressHex(hex_value);
			
			// run the results as a standard command
			sendCommand('securePacket', unsecure_params, false, files);
		}
		
		private static function loaderHandler(e:Event):void
		{
			var loader:URLLoader = URLLoader(e.target);
			// when we get input, we can dump it for API developers to debug with
			echo("INPUT: \n" + loader.data + "\n");
			
			for (var i:uint = 0; i < loaders.length; i++)
			{
				if (loaders[i] == loader)
				{
					loaders.splice(i, 1);
					break;
				}
			}
			
			var response:Object;
			if (loader.data) {
				// decode the server response
				response = JSON_.decode(loader.data);
			} else {
				response = {success:false};
			}
			
			// if the command was unsuccessful we'll pass the error to the author
			if (!response.success) {
				var error:NewgroundsAPIError = new NewgroundsAPIError(response.error_code, response.error_msg);
				sendError(response, error);
				
			// if all is well, we'll let our event handling take over
			} else {
				doEvent(response);
			}
		}
		
		// This function passes commands to the API Gateway
		private static function sendCommand(command:String, params:Object, open_browser:Boolean = false, files:Object = null):void
		{
			// make sure connectMovie has been called before any other calls can be sent to the gateway
			if (!connected && command != "connectMovie") {
				var msg:String = "NewgroundsAPI."+command+"() - NewgroundsAPI.connectMovie() must be called before this command can be called\n";
				fatalError(msg,'connectMovie');
			}
			
			// set a container for variables to be passed to the gateway based on the method being used
			var output:URLVariables = new URLVariables();
					
			// set expected variables
			output.command_id = getCommandID(command);
			output.tracker_id = movie_id;		
			
			// pass along debug if we're using it
			if (debug) {
				output.debug = debug;
			}
			
			var i:String; // TODO
	
			// pass any additional parameters as varibles
			if (params) {
				for(i in params) {
					output[i] = params[i];
				}
			}
			
			// pass data for any files
			if (files) {
				for(i in files) {
					output[i] = files[i];
				}
			}
	
			// dump the output object for API developer debugging
			echo("OUTPUT: \n" + JSON_.encode(output) + "\n");
			
			var urlRequest:URLRequest = new URLRequest(GATEWAY_URL + "?seed=" + Math.random());
			urlRequest.data = output;
			
			// some commands are used to load web pages, so they are passed in a new browser window
			if (open_browser) {
				urlRequest.method = URLRequestMethod.GET;
				urlRequest.data = output;
				
				navigateToURL(urlRequest, "_blank");
				
			// most commands get passed direct to the gateway and wait for a response
			} else {					
				var op:Array = new Array();
				for(var o:String in output) {
					op.push(o+"="+escape(output[o]));
				}
				echo("POST " + GATEWAY_URL + "?" + op.join("&"));
				
				urlRequest.method = URLRequestMethod.POST;
				
				var loader:URLLoader = new URLLoader();
				loader.addEventListener(Event.COMPLETE, loaderHandler);
				loaders.push(loader);
				loader.load(urlRequest);
				trace(urlRequest.data);
			}
		}
		
		//===================================== Flash Ad Generation ===================================\\
		
		private static function renderAd(target:DisplayObjectContainer):void
		{
			var adRect:Shape;
			var mask:Shape;
			
			if (ad) removeAd();						// clean up previous ad if necessary
			
			if (ad_swf_url) {
				adRect = new Shape();
				adRect.graphics.beginFill(0x000000);
				adRect.graphics.moveTo(0,0);
				adRect.graphics.lineTo(300,0);
				adRect.graphics.lineTo(300,250);
				adRect.graphics.lineTo(0,250);
				adRect.graphics.lineTo(0,0);
				adRect.graphics.endFill();
				
				mask = new Shape();
				mask.graphics.beginFill(0x000000);
				mask.graphics.moveTo(0,0);
				mask.graphics.lineTo(300,0);
				mask.graphics.lineTo(300,250);
				mask.graphics.lineTo(0,250);
				mask.graphics.lineTo(0,0);
				mask.graphics.endFill();
		
				ad = new Loader();
				
				adContainer = new Sprite();
				adContainer.addChild(adRect);
				adContainer.addChild(ad);
				adContainer.addChild(mask);
				ad.mask = mask;
				target.addChild(adContainer);
				
				// REMOVED_FROM_STAGE added in 9.0.28.0. Fail silently in earlier versions
				if (isFlashVersion(9, 0, 28))
					adContainer.addEventListener(REMOVED_FROM_STAGE, removeAdHandler);
				
				ad.load(new URLRequest(ad_swf_url));
				callListener(events.AD_ATTACHED, true, null, target);
			} else {
				callListener(events.AD_ATTACHED, false, new NewgroundsAPIError("FLASH_ADS_NOT_APPROVED","Unable to render ad"));
			}
		}
		
		private static function adLoaderHandler(e:Event):void
		{
			var loader:URLLoader = URLLoader( e.target );
			
			if (loader.data) {
				ad_swf_url = String(loader.data);
			} else {
				ad_swf_url = null;
			}

			renderAd(flashAdTarget);
		}
				
		private static function ioErrorHandler(e:IOErrorEvent):void
		{
			sendWarning("Ad failed to load:" + e.toString());
		}
		
		// if ads are approved and not blocked, this loads a flash ad in the target movieclip
		public static function attachFlashAd(target:DisplayObjectContainer):void
		{
			Security.allowDomain("http://server.cpmstar.com");
			Security.allowDomain("http://www.cpmstar.com");
			Security.allowDomain("https://server.cpmstar.com");
			Security.allowDomain("https://www.cpmstar.com");
			Security.allowInsecureDomain("http://server.cpmstar.com");
			Security.allowInsecureDomain("http://www.cpmstar.com");
			Security.allowInsecureDomain("https://server.cpmstar.com");
			Security.allowInsecureDomain("https://www.cpmstar.com");
			
			flashAdTarget = target;
			
			sendMessage("You may get a security sandbox violation from this ad.  This is nothing to worry about!");
			// if the timer has been reset, load a new ad_swf_url
			if (resetAdTimer()) {
				if (ad_url) {
					adURLLoader = new URLLoader();
					var urlRequest:URLRequest;
					adURLLoader.addEventListener(Event.COMPLETE, adLoaderHandler);
					adURLLoader.addEventListener(IOErrorEvent.IO_ERROR, ioErrorHandler);
					
					// load the ad with a random seed to avoid caching
					if (ad_url.indexOf('?') > -1) {
						urlRequest = new URLRequest(ad_url+"&random="+Math.random());
						adURLLoader.load(urlRequest);
					} else {
						urlRequest = new URLRequest(ad_url + "?random=" + Math.random());
						adURLLoader.load(urlRequest);
					}
				}
			// use the current ad_swf_url
			} else {
				renderAd(target);
			}
		}
		
		// this is used so requests to the actual ad server only happen once every 5 minutes
		private static function resetAdTimer():Boolean
		{
			// if we don't have an ad_url yet, leave the timer alone
			if (!ad_url) {
				return false;
			}
			
			var d:Date = new Date();
			
			// if the time has expired, update the reset timer and return true
			if (d.getTime() >= ad_reset) {
				ad_reset = d.getTime() + (1000*60*5); // 5 minutes
				return true;
			}
			
			// time hasn't expired
			return false;
		}
	
		private static function removeAdHandler(e:Event):void
		{
			removeAd();
		}
		
		public static function removeAd():void
		{
			if (adURLLoader)
			{
				try { adURLLoader.close(); } // close throws an error if loading is done
				catch (e:Error) { }
				
				adURLLoader = null;
			}
				
			if (ad)
			{
				try { ad.close(); }
				catch(e:Error) { }
				
				// MIKE: unloadAndStop was only added in FP10. Calling it in FP9-targetted Flash results in an undefined function error.
				try { Object(ad).unloadAndStop(true); trace("uas"); }		// cast to Object to avoid compile time error
				catch (e:Error) { ad.unload(); }							// catch run-time reference errors. Default to unload()
				
				if (ad.parent)
					ad.parent.removeChild(ad);
			}
			
			if (adContainer)
			{
				// MIKE: REMOVED_FROM_STAGE added in 9.0.28, avoid error if earlier
				if (isFlashVersion(9,0,28,0))
					adContainer.removeEventListener(REMOVED_FROM_STAGE, removeAdHandler);
					
				if (adContainer.parent)
				{
					adContainer.parent.removeChild(adContainer);
				}
			}
				
			ad = null;
			adContainer = null;
		}
		
		//===================================== Debugging ============================================\\
		
		// this function passes information to the author for debugging
		private static function sendMessage(m:String, r:Boolean = false):String
		{
			var msg:String = "[NewgroundsAPI] :: "+m;
			if (r) {
				return(msg);
			} else {
				trace(msg);
				return null;
			}
		}
		
		// this function passes information for API Developers if do_echo is true
		private static function echo(m:String):void
		{
			if (do_echo) {
				trace(m);
			}
		}
		
		public static function isFlashVersion(major:uint, minor:uint = 0, buildNumber:uint = 0, internalBuildNumber:uint = 0):Boolean
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
	}
}