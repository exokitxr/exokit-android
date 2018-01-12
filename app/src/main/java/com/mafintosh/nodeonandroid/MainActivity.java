package com.mafintosh.nodeonandroid;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.content.LocalBroadcastManager;
import android.os.Bundle;
import android.webkit.WebView;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.net.InetAddress;
// import java.net.ServerSocket;
// import java.net.Socket;
// import java.util.Arrays;

import org.json.*;

public class MainActivity extends Activity {
    private WebView browser;
    private NodeReceiver receiver;

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Intent i = new Intent(MainActivity.this, NodeService.class);
        stopService(i);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // setContentView(R.layout.activity_main);
        WebView webview = new WebView(this);
        setContentView(webview);

        receiver = new NodeReceiver();
        IntentFilter filter = new IntentFilter("com.mafintosh.nodeonandroid.ipc");
        LocalBroadcastManager.getInstance(this).registerReceiver(receiver, filter);

        // browser = (WebView) findViewById(R.id.webview);
        browser = webview;
        browser.getSettings().setLoadWithOverviewMode(true);
        browser.getSettings().setUseWideViewPort(true);
        browser.getSettings().setJavaScriptEnabled(true);

        final Context me = this;

        new Thread(
                new SimpleWebServer() {
                    @Override
                    public void onStart() {
                        System.out.println("web server start 1");

                        Intent i = new Intent(MainActivity.this, NodeService.class);
                        i.putExtra("ipc-port", "" + this.getLocalPort());
                        startService(i);

                        System.out.println("web server start 2");
                    }

                    public byte[] handleRequest(String routeString, String requestString) {
                        System.out.println("web server request " + routeString);

                        try {
                            JSONObject obj = new JSONObject(requestString);
                            String method = obj.getString("method");
                            String args = obj.getString("args");

                            Intent in = new Intent("com.mafintosh.nodeonandroid.ipc");
                            in.putExtra("method", method);
                            in.putExtra("args", args);
                            LocalBroadcastManager.getInstance(me).sendBroadcast(in);

                            return new byte[0];

                            /* if (method.equals("loadUrl")) {
                              if (args != null) {
                                browser.loadUrl(args);

                                return new byte[0];
                              } else {
                                System.err.println("java api loadUrl method got invalid args: " + args);

                                return new byte[0];
                              }
                            } else {
                              System.err.println("java api got invalid method: " + method);

                              return new byte[0];
                            } */
                        } catch (Exception err) {
                            err.printStackTrace();

                            return new byte[0];
                        }
                    }
                }
        ).start();

        /* new Thread(
                new Runnable() {
                    @Override
                    public void run() {
                        try {
                            ServerSocket server = new ServerSocket(0, 5, InetAddress.getByName("127.0.0.1"));

                            Intent i = new Intent(MainActivity.this, NodeService.class);
                            i.putExtra("ipc-port", "" + server.getLocalPort());
                            startService(i);

                            Socket socket = server.accept();
                            BufferedInputStream inp = new BufferedInputStream(socket.getInputStream());
                            BufferedOutputStream out = new BufferedOutputStream(socket.getOutputStream());

                            byte[] buf = new byte[65536];

                            while (true) {
                                int read = inp.read(buf);
                                String u = new String(Arrays.copyOfRange(buf, 0, read));
                                Intent in = new Intent("com.mafintosh.nodeonandroid.ipc");
                                in.putExtra("loadUrl", u);
                                LocalBroadcastManager.getInstance(me).sendBroadcast(in);
                            }
                        } catch (Exception err) {
                            err.printStackTrace();
                        }
                    }
                }
        ).start(); */

    }

    private class NodeReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String method = intent.getStringExtra("method");
            String args = intent.getStringExtra("args");

            if (method != null) {
              if (method.equals("loadUrl")) {
                if (args != null) {
                  browser.loadUrl(args);
                } else {
                  System.err.println("java api loadUrl method got invalid args: " + args);
                }
              } else if (method.equals("enterVr") || method.equals("enterAr")) {
                Intent in = new Intent(MainActivity.this, TreasureHuntActivity.class);
                MainActivity.this.startActivity(in);
              } else {
                System.err.println("java api got invalid method: " + method);
              }
            } else {
              System.err.println("java api got missing metod");
            }
        }
    }
}
