package com.mafintosh.nodeonandroid;

import android.app.Service;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.IBinder;
import android.support.annotation.Nullable;
import android.content.Context;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class NodeService {
    static {
      System.loadLibrary("node");
      System.loadLibrary("nodebinding");
    }

    public static void init(Context context) {
      String cache = context.getCacheDir().getAbsolutePath();
      String nodePath = cache + "/node";
      String corePath = cache + "/node_modules";
      AssetManager am = context.getAssets();
      copyAssets(am, "node_modules", corePath);
      copyAssets(am, "node", nodePath);
      // startNode("node", jsPath, "" + ipcPort);
      // startNode(nodePath + "/html5.js", "" + 8000);
      start(nodePath + "/html5.js", "" + 8000);
    }
    public static native void tick();

    // private static native void startNode(String scriptString, String portString);
    private static native void start(String scriptString, String portString);

    private static void copyAssets (AssetManager am, String src, String dest) {
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
