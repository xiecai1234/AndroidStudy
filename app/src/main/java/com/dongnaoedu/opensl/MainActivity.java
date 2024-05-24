package com.dongnaoedu.opensl;

import java.io.File;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.EditText;

import com.cxp.androidstudy.R;

public class MainActivity extends Activity {

	private EditText et_file;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_opensl);
		et_file = (EditText) findViewById(R.id.et_file);
	}

	/**
	 * 播放音频文件
	 * @param btn
	 */
	public void mPlay(View btn){
		new Thread(){
			public void run() {
				String fileName = et_file.getText().toString();
				File file = new File(Environment.getExternalStorageDirectory(),fileName);
				AudioPlayer.play(file.getAbsolutePath());
			}
		}.start();
	}
	
}
