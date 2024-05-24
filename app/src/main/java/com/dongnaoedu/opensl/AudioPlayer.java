package com.dongnaoedu.opensl;

public class AudioPlayer {

	public native static void play(String filePath);
	
	static{
		System.loadLibrary("JasonAudioPlayer");
	}
}
