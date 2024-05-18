package com.cxp.androidstudy;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import com.cxp.androidstudy.databinding.ActivityHelloworldBinding;

public class HelloWorldActivity extends AppCompatActivity {

    // Used to load the 'androidstudy' library on application startup.
    static {
        System.loadLibrary("androidstudy");
    }

    private ActivityHelloworldBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityHelloworldBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());
    }

    /**
     * A native method that is implemented by the 'androidstudy' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}