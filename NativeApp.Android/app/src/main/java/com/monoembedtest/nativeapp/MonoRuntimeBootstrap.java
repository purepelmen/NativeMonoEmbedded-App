package com.monoembedtest.nativeapp;

import android.content.res.AssetManager;

public class MonoRuntimeBootstrap {

    public static native int start(String exePath, AssetManager assetManager);
}
