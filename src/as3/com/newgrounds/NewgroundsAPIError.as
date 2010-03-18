package com.newgrounds
{
	import flash.utils.Dictionary;
	public class NewgroundsAPIError
	{
		public static const aliases:Array = new Array(
				"UNKNOWN_ERROR",
				"INVALID_API_ID",
				"MISSING_PARAM",
				"INVALID_STAT_ID",
				"INVALID_COMMAND_ID",
				"FLASH_ADS_NOT_APPROVED",
				"PERMISSION_DENIED",
				"IDENTIFICATION_REQUIRED",
				"INVALID_EMAIL_ADDRESS",
				"BANNED_USER",
				"SESSION_EXPIRED",
				"INVALID_SCORE",
				"INVALID_MEDAL",
				"INVALID_FOLDER",
				"FILE_NOT_FOUND",
				"SITE_ID_REQUIRED",
				"UPLOAD_IN_PROGRESS",
				"USER_CANCELLED",
				"CONFIRM_REQUEST",
				"CONNECTION_FAILED"
			);
		
		// used when converting aliases to clean/formatted error names.
		private static const always_caps:Array = new Array(
			"API",
			"URL",
			"ID"
		);
		
		// creates an dictionary of error codes keyed on alias names
		public static function init_codes():Dictionary
		{
			var result:Dictionary = new Dictionary();
			for (var i:uint = 0; i < aliases.length; i++)
			{
				result[aliases[i]] = i;
			}
			
			return result;
		}
		
		private static function init_names():Array
		{
			var result:Array = new Array();
			for (var i:uint = 0; i < aliases.length; i++)
			{
				var alias_parts:Array = aliases[i].toLowerCase().split("_");

				for(var a:uint=0; a<alias_parts.length; a++) {
					alias_parts[a] = alias_parts[a].substr(0,1).toUpperCase() + alias_parts[a].substr(1, alias_parts[a].length);
					for each(var c:String in always_caps)
					{
						if (alias_parts[a].toUpperCase() == c) {
							alias_parts[a] = alias_parts[a].toUpperCase();
						}
					}
				}
				
				result[i] = alias_parts.join(" ");
			}
			
			return result;
		}

		
		public static const error_codes:Object = init_codes();
		public static const error_names:Object = init_names();
		
		public var code:Number = 0;
		public var message:String;
		public var name:String;
		public var alias:String;
		
		public function NewgroundsAPIError(error:*, msg:String) 
		{
			if (error is String) {
				error = error_codes[error];
			} else if(!(error is uint)) {
				error = 0;
			}
			trace(error);
			code = error;
			message = msg;
			name = error_names[error];
			trace(name);
			alias = aliases[error];
		}
		
		public function isError():Boolean
		{
			return true;
		}
	}
}