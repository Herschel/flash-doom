package com.newgrounds
{
	import flash.utils.Dictionary;
	
	public class BaseN {
		
		private var _hashIndex:String;
		private var	_hashVal:Dictionary;
		private var	_base:Number;
		
		public function BaseN(hash:String = null)
		{			
			if (hash) {
				_hashIndex = hash;
			} else {
				// this base character set uses characters that are linux comapatable and do not conflict with most object notations
				_hashIndex = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`~@#$%^&*()+|;/";
			}
			
			_base = _hashIndex.length;
			
			// capture the hash values to a dictionary
			_hashVal = new Dictionary();
			for (var i:uint = 0; i < _base; i++)
				_hashVal[_hashIndex.charAt(i)] = i;

		}
		
		public function encode(n:Number, minchars:uint = 1):String
		{
			var s:String = n.toString();
			var str:String = "";
			
			if (s.charAt(0) == "-")
			{
				str = "-";
				s = s.substring(1);
			}

			var halves:Array = s.split(".", 2);
			
			str += baseNEncoder(halves[0], minchars);
			
			// if we have a decimal
			if (halves.length > 1)
			{
				str += "." + baseNEncoder(halves[1]);
			}
			return str;
		}
		
		
		public function decode(s:String):Number
		{
			var val:String = "";
			if (s.charAt(0) == "-")
			{
				val = "-";
				s = s.substring(1);
			}
			
			var halves:Array = s.split(".", 2);
			val += baseNDecoder(halves[0]);
			
			// if we have a decimal
			if (halves.length > 1)
			{
				val += "."
				val += baseNDecoder(halves[1]);
			}
			
			return Number(val);
		}
		
		
		private function baseNEncoder(n:uint, minchars:uint = 1):String
		{
			var str:String = "";
			var val:uint = n;
			
			while (val != 0)
			{
				str = _hashIndex.charAt(val % _base) + str;
				val /= _base;
			}
			
			if (minchars)
			{
				while (str.length < minchars)
					str = _hashIndex.charAt(0) + str;
			}
			
			return str;
		}
		
		private function baseNDecoder(s:String):uint {
			var val:uint = 0;
			
			for (var i:uint = 0; i < s.length; i++)
			{
				val *= _base;
				val += _hashVal[s.charAt(i)];
			}
			
			return val;
		}
	}
}