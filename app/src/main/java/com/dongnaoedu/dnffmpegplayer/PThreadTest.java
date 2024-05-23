package com.dongnaoedu.dnffmpegplayer;

public class PThreadTest {
	public native void test1();
	public native void test2();
	public native void test3();

	
	static{
		System.loadLibrary("test_thread");
	}
}
