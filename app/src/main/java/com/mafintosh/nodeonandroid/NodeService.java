package com.mafintosh.nodeonandroid;

import android.app.Service;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.content.Context;
import android.util.Log;
import android.util.TimingLogger;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

public class NodeService {
    private static final String TAG = NodeService.class.getSimpleName();

    private Context context;

    static {
      System.loadLibrary("node");
      System.loadLibrary("nodebinding");
    }

    public NodeService(Context ctx) {
      context = ctx;
    }

    public void init(String url, String vrMode, int vrTexture) {
      String nativeLibraryDir = context.getApplicationInfo().nativeLibraryDir;

      {
        TimingLogger timings = new TimingLogger(TAG, "node service init");

        String cache = context.getCacheDir().getAbsolutePath();
        String nodePath = cache + "/node";
        String corePath = cache + "/node_modules";
        AssetManager am = context.getAssets();
        copyAssets(am, "node_modules", corePath);
        copyAssets(am, "node", nodePath);

        Log.i(TAG, "copied assets");
        timings.addSplit("copied assets");

        start(nativeLibraryDir + "/node.so", nodePath + "/html5.js", nativeLibraryDir, am, url, vrMode, vrTexture);

        Log.i(TAG, "started node");
        timings.addSplit("started node");

        timings.dumpToLog();
      }
    }

    // uv
    private native void start(String binString, String scriptString, String libPath, AssetManager am, String url, String vrMode, int vrTexture);
    public native void tick(int timeout);

    // GL
    // public native void onSurfaceCreated();
    public native void onResize(int width, int height);

    // VR
    public native void onNewFrame(float[] headViewMatrix, float[] headQuaternion);
    public native void onDrawEye(float[] eyeViewMatrix, float[] eyePerspectiveMatrix);

    // AR
    public native void onDrawFrame(float[] viewMatrix, float[] projectionMatrix, float[] centerArray);

    // helpers
    private static void copyAssets(AssetManager am, String src, String dest) {
        try {
            copyAssetFile(am, src, dest);
        } catch (Exception e) {
            try {
                File dir = new File(dest);
                dir.mkdir();
            } catch (Exception e1) {}
            try {
                String[] files = am.list(src);
                for (int i = 0; i < files.length; i++) {
                    copyAssets(am, src + "/" + files[i], dest + "/" + files[i]);
                }
            } catch (Exception e2) {}
        }
    }

    private static void copyAssetFile(AssetManager am, String src, String dest) throws IOException {
        InputStream in = am.open(src);

        File destFile = new File(dest);
        if (!destFile.exists()) destFile.createNewFile();

        FileOutputStream out = new FileOutputStream(dest);

        byte[] buffer = new byte[1024];
        int length;
        while ((length = in.read(buffer)) > 0) {
            out.write(buffer, 0, length);
        }
        in.close();
        out.close();
    }
}
