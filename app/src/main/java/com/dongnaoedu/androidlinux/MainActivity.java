package com.dongnaoedu.androidlinux;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;

import com.cxp.androidstudy.R;

public class MainActivity extends Activity {

	private PosixThread p;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		p = new PosixThread();
		//子线程里init报错 Fatal signal 6 (SIGABRT), code -1 (SI_QUEUE) in tid 24716 (my_thread), pid 24587 (xp.androidstudy)
//		new Thread(new Runnable() {
//			@Override
//			public void run() {
//				p.init();
//			}
//		});
		p.init();
	}

	public void mTest(View btn){
		p.pthread();
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		p.destroy();
	}
}
