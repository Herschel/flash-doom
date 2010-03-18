package com.newgrounds 
{
	
	/**
	 * ...
	 * @author ...
	 */
	public class Medal 
	{
		private var _id:uint;
		private var _name:String;
		private var _unlocked:Boolean;
		private var _value:uint;
		
		public function Medal(id:uint, name:String, unlocked:Boolean, _value:uint) 
		{
			_id = id;
			_name = name;
			_unlocked = unlocked;
			_value = value;
		}
		
		public function get id():uint			{ return _id; }
		public function get name():String		{ return _name; }
		public function get unlocked():Boolean	{ return _unlocked; }
		public function get value():Boolean		{ return _value; }
		
	}
	
}