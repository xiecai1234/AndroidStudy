package com.dongnaoedu.dnffmpegplayer;

import java.io.File;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;

import com.cxp.androidstudy.R;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
	}

	public void mDecode(View btn){
		String input = new File(Environment.getExternalStorageDirectory(),"mvtest.mp4").getAbsolutePath();
		String output = new File(Environment.getExternalStorageDirectory(),"output.yuv").getAbsolutePath();
		VideoUtils.decode(input, output);
	}
}
