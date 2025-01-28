package com.monoembedtest.nativeapp;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.IOException;
import java.util.Locale;

public class MainActivity extends Activity {

    public static final String MANAGED_DIR = "Managed";

    static {
        System.loadLibrary("nativeapp");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        AssetsExtractor extractor = AssetsExtractor.create(this);
        try {
            extractor.extract(MANAGED_DIR, MANAGED_DIR);
        } catch (IOException e) {
            Log.e("MainActivity", "Error extracting embedded Managed app files.", e);
        }

        int retCode = MonoRuntimeBootstrap.start(getCacheDir().toString(), getAssets());

        // Reporting information to the user.
        super.<TextView>findViewById(R.id.sample_text).setText(String.format(Locale.ENGLISH,
                "The native side has returned control and the retcode is: %d." ,
                retCode));
    }
}