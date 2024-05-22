package com.dongnaoedu.dnffmpegplayer;

public class PThreadTest {
	public native void test();

	
	static{
		System.loadLibrary("test_thread");
	}
}
