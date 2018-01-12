/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
package com.mafintosh.nodeonandroid;
 
import android.content.res.AssetManager;
import android.text.TextUtils;
import android.util.Log;
 
import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.InetAddress;
 
/**
 * Implementation of a very basic HTTP server. The contents are loaded from the assets folder. This
 * server handles one request at a time. It only supports GET method.
 */
public class SimpleWebServer implements Runnable {
 
    private static final String TAG = "SimpleWebServer"; 
 
    /**
     * True if the server is running.
     */
    private boolean mIsRunning = true;
 
    /**
     * The {@link java.net.ServerSocket} that we listen to.
     */
    private ServerSocket mServerSocket;
 
    /**
     * WebServer constructor.
     */
    public SimpleWebServer() {}
 
    /**
     * This method stops the web server
     */
    public void stop() {
        try {
            mIsRunning = false;
            if (null != mServerSocket) {
                mServerSocket.close();
                mServerSocket = null;
            }
        } catch (IOException e) {
            Log.e(TAG, "Error closing the server socket.", e);
        }
    }
 
    public int getLocalPort() {
      return mServerSocket.getLocalPort();
    }
 
    @Override
    public void run() {
        try {
            mServerSocket = new ServerSocket(0, 5, InetAddress.getByName("127.0.0.1"));
            onStart();
            while (mIsRunning) {
                Socket socket = mServerSocket.accept();
                handle(socket);
                socket.close();
            }
        } catch (SocketException e) {
            // The server was stopped; ignore.
        } catch (IOException e) {
            Log.e(TAG, "Web server error.", e);
        }
    }
 
    /**
     * Respond to a request from a client.
     *
     * @param socket The client socket.
     * @throws IOException
     */
    private void handle(Socket socket) throws IOException {
        BufferedReader reader = null;
        PrintStream output = null;
        try {
            String routeString = null;
            String requestString = null;
            Boolean headersDone = false;

            System.out.println("socket start read");
 
            // Read HTTP headers and parse out the route.
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            while (true) {
                System.out.println("socket reading line...");
                String line = reader.readLine();
                System.out.println("socket got line : " + line);
                if (line != null) {
                  if (!headersDone) {
                    if (line.startsWith("POST /")) {
                        int start = line.indexOf('/') + 1;
                        int end = line.indexOf(' ', start);
                        routeString = line.substring(start, end);

                        System.out.println("socket route done");
                    } else if (line.isEmpty()) {
                      System.out.println("socket headers done");

                      headersDone = true;
                    }
                  } else {
                    requestString = line;

                    System.out.println("socket reading done normally");

                    break;
                  }
                } else {
                  System.out.println("socket reading done abruptly");

                  break;
                }
            }

            System.out.println("socket handle");
 
            // Output stream that we send the response to
            output = new PrintStream(socket.getOutputStream());
 
            // Prepare the content to send.
            if (null == routeString) {
                writeServerError(output);
                return;
            }
            byte[] bytes = handleRequest(routeString, requestString);
            if (null == bytes) {
                writeServerError(output);
                return;
            }
 
            // Send out the content.
            output.println("HTTP/1.0 200 OK");
            output.println("Content-Type: " + "application/javascript");
            output.println("Content-Length: " + bytes.length);
            output.println();
            output.write(bytes);
            output.flush();
        } finally {
            if (null != output) {
                output.close();
            }
            if (null != reader) {
                reader.close();
            }
        }
    }
 
    /**
     * Writes a server error response (HTTP/1.0 500) to the given output stream.
     *
     * @param output The output stream.
     */
    private void writeServerError(PrintStream output) {
        output.println("HTTP/1.0 500 Internal Server Error");
        output.flush();
    }

    protected void onStart() {}

    protected byte[] handleRequest(String routeString, String requestString) {
      return new byte[0];
    }
 
}
