package com.dongnaoedu.androidlinux;

public class PosixThread {

	public native void pthread();
	
	public native void init();
	
	public native void destroy();
	
	static{
		System.loadLibrary("dn_android_linux");
	}
}
