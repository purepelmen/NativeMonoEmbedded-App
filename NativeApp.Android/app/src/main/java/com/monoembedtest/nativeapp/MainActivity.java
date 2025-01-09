package com.monoembedtest.nativeapp;

import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
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

        try {
            extractAssetsToCache(MANAGED_DIR, MANAGED_DIR);
        } catch (IOException e) {
            Log.e("MainActivity", "Error extracting embedded Managed app files.", e);
        }

        int retCode = MonoRuntimeBootstrap.start(getCacheDir().toString(), getAssets());

        // Reporting information to the user.
        super.<TextView>findViewById(R.id.sample_text).setText(String.format(Locale.ENGLISH,
                "The native side has returned control and the retcode is: %d." ,
                retCode));
    }

    private void extractAssetsToCache(String sourceFolder, String destPath) throws IOException {
        File destinationDir = new File(getCacheDir(), destPath);
        File dontReextractFile = new File(destinationDir, ".dont_reextract");

        // The dest folder already exists and the dontreextract file too? Then skip.
        if (!destinationDir.mkdirs() && dontReextractFile.exists()) {
            Log.v("MainActivity", "Skipping assets extraction because already extracted: " + destinationDir);
            return;
        }

        AssetManager assetManager = getAssets();
        String[] contents = assetManager.list(sourceFolder);

        if (contents == null) {
            throw new IllegalArgumentException("source doesn't exist in the assets");
        }

        byte[] buffer = new byte[1024];
        for (String fileName : contents) {
            String readFilePath = sourceFolder + "/" + fileName;

            try (InputStream in = assetManager.open(readFilePath);
                 FileOutputStream out = new FileOutputStream(new File(destinationDir, fileName))) {

                int read;
                while ((read = in.read(buffer)) != -1) {
                    out.write(buffer, 0, read);
                }
            }
        }

        dontReextractFile.createNewFile();
    }
}