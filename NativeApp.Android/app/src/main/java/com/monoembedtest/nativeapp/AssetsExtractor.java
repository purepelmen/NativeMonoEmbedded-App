package com.monoembedtest.nativeapp;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;

public class AssetsExtractor {

    private enum FolderState
    {
        UNCHANGED,
        UPDATED,
        MISSING
    }

    private final AssetManager assetManager;
    private final File destBasePath;

    private final long lastUpdateTime;

    public static AssetsExtractor create(Context context) {
        return new AssetsExtractor(context.getAssets(), context.getCacheDir(), getPackageInfo(context).lastUpdateTime);
    }

    private static PackageInfo getPackageInfo(Context context) {
        try {
            return context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
        } catch (PackageManager.NameNotFoundException e) {
            throw new RuntimeException("Unexpected situation: failed to get own package info.", e);
        }
    }

    public AssetsExtractor(AssetManager assetManager, File destBasePath, long lastUpdateTime) {
        if (assetManager == null)
            throw new NullPointerException("assetManager is null");
        if (destBasePath == null || !destBasePath.isDirectory())
            throw new IllegalArgumentException("destBasePath is null, does not exists, or is not a directory");

        this.assetManager = assetManager;
        this.destBasePath = destBasePath;

        this.lastUpdateTime = lastUpdateTime;
    }

    public void extract(String sourceFolder, String destPath) throws IOException {
        File destinationDir = new File(destBasePath, destPath);
        File lastUpdateFile = new File(destinationDir, ".last_update");

        FolderState state = checkForUpdate(lastUpdateFile);
        if (state == FolderState.UNCHANGED) {
            Log.v("AssetExtractor", "Skipping assets extraction because already extracted: " + destinationDir);
            return;
        }

        if (state == FolderState.UPDATED) {
            Log.i("AssetExtractor", "Update detected and will be done for folder: " + destinationDir);
        }

        destinationDir.mkdirs();
        extractAssetsSubdir(sourceFolder, destinationDir);

        writeLastUpdateFile(lastUpdateFile);
    }

    private FolderState checkForUpdate(File lastUpdateFile) {
        if (!lastUpdateFile.isFile()) {
            return FolderState.MISSING;
        }

        long lastFolderUpdate = 0;

        try (BufferedReader bufferedReader = new BufferedReader(new FileReader(lastUpdateFile))) {
            lastFolderUpdate = Long.parseLong(bufferedReader.readLine());
        } catch (IOException exception) {
            throw new RuntimeException(exception);
        } catch (NumberFormatException exception) {
            Log.e("AssetExtractor", "'.last_update' file has incorrect data.");

            lastUpdateFile.delete();
            return FolderState.MISSING;
        }

        return lastUpdateTime == lastFolderUpdate ? FolderState.UNCHANGED : FolderState.UPDATED;
    }

    private void writeLastUpdateFile(File lastUpdateFile) {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(lastUpdateFile, false))) {
            writer.write(Long.toString(lastUpdateTime));
            writer.newLine();
        }
        catch (IOException exception) {
            throw new RuntimeException(exception);
        }
    }

    private void extractAssetsSubdir(String targetSubdir, File destDir) throws IOException {
        String[] contents = assetManager.list(targetSubdir);
        if (contents == null) {
            throw new IllegalArgumentException("source doesn't exist in the assets");
        }

        byte[] buffer = new byte[1024];
        for (String childEntry : contents) {
            String fullReadPath = targetSubdir + "/" + childEntry;
            File destEntry = new File(destDir, childEntry);

            InputStream targetDirOrFile;
            try {
                targetDirOrFile = assetManager.open(fullReadPath);
            }
            catch (FileNotFoundException exception) {
                destEntry.mkdir();
                extractAssetsSubdir(fullReadPath, destEntry);
                continue;
            }

            try (InputStream inSource = targetDirOrFile; FileOutputStream outDest = new FileOutputStream(destEntry)) {
                int read;
                while ((read = inSource.read(buffer)) != -1) {
                    outDest.write(buffer, 0, read);
                }
            }
        }
    }
}
