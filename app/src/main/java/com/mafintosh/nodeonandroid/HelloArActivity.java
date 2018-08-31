/*
 * Copyright 2017 Google Inc. All Rights Reserved.
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

import android.opengl.GLES11Ext;
import android.opengl.GLES30;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MotionEvent;
import android.widget.Toast;
import com.google.ar.core.Anchor;
import com.google.ar.core.ArCoreApk;
import com.google.ar.core.Camera;
import com.google.ar.core.Frame;
import com.google.ar.core.HitResult;
import com.google.ar.core.Plane;
import com.google.ar.core.Point;
import com.google.ar.core.Pose;
import com.google.ar.core.Point.OrientationMode;
import com.google.ar.core.PointCloud;
import com.google.ar.core.Session;
import com.google.ar.core.Trackable;
import com.google.ar.core.TrackingState;
import com.mafintosh.nodeonandroid.helpers.CameraPermissionHelper;
import com.mafintosh.nodeonandroid.helpers.DisplayRotationHelper;
import com.mafintosh.nodeonandroid.helpers.FullScreenHelper;
import com.mafintosh.nodeonandroid.helpers.SnackbarHelper;
import com.mafintosh.nodeonandroid.helpers.TapHelper;
import com.mafintosh.nodeonandroid.rendering.BackgroundRenderer;
import com.mafintosh.nodeonandroid.rendering.ObjectRenderer;
import com.mafintosh.nodeonandroid.rendering.ObjectRenderer.BlendMode;
import com.mafintosh.nodeonandroid.rendering.PlaneRenderer;
import com.mafintosh.nodeonandroid.rendering.PointCloudRenderer;
import com.google.ar.core.exceptions.CameraNotAvailableException;
import com.google.ar.core.exceptions.UnavailableApkTooOldException;
import com.google.ar.core.exceptions.UnavailableArcoreNotInstalledException;
import com.google.ar.core.exceptions.UnavailableDeviceNotCompatibleException;
import com.google.ar.core.exceptions.UnavailableSdkTooOldException;
import com.google.ar.core.exceptions.UnavailableUserDeclinedInstallationException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.concurrent.ArrayBlockingQueue;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * This is a simple example that shows how to create an augmented reality (AR) application using the
 * ARCore API. The application will display any detected planes and will allow the user to tap on a
 * plane to place a 3d model of the Android robot.
 */
public class HelloArActivity extends AppCompatActivity implements GLSurfaceView.Renderer {
    private static int FRAME_TIME_MAX = 1000 / 60;
    private static int FRAME_TIME_MIN = FRAME_TIME_MAX / 5;
    private static final String TAG = HelloArActivity.class.getSimpleName();

    // Rendering. The Renderers are created here, and initialized when the GL surface is created.
    private GLSurfaceView surfaceView;

    private boolean installRequested;

    private Session session;
    private final SnackbarHelper messageSnackbarHelper = new SnackbarHelper();
    private DisplayRotationHelper displayRotationHelper;
    private TapHelper tapHelper;
    private NodeService service;
    boolean serviceInitialized;
    private int mTextureId;
    long lastFrameTime;
    private Anchor mAnchor = null;


    private final BackgroundRenderer backgroundRenderer = new BackgroundRenderer();
    private final ObjectRenderer virtualObject = new ObjectRenderer();
    private final ObjectRenderer virtualObjectShadow = new ObjectRenderer();
    private final PlaneRenderer planeRenderer = new PlaneRenderer();
    private final PointCloudRenderer pointCloudRenderer = new PointCloudRenderer();

    // Temporary matrix allocated here to reduce number of allocations for each frame.
    private final float[] anchorMatrix = new float[16];
    private static final float[] DEFAULT_COLOR = new float[] {0f, 0f, 0f, 0f};

    // Anchors created from taps used for object placing with a given color.
    private static class ColoredAnchor {
        public final Anchor anchor;
        public final float[] color;

        public ColoredAnchor(Anchor a, float[] color4f) {
            this.anchor = a;
            this.color = color4f;
        }
    }

    private final ArrayList<ColoredAnchor> anchors = new ArrayList<>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = findViewById(R.id.surfaceview);
        displayRotationHelper = new DisplayRotationHelper(/*context=*/ this);

        // Set up tap listener.
        tapHelper = new TapHelper(/*context=*/ this);
        surfaceView.setOnTouchListener(tapHelper);

        // Set up renderer.
        surfaceView.setPreserveEGLContextOnPause(true);
        surfaceView.setEGLContextClientVersion(2);
        surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0); // Alpha used for plane blending.
        surfaceView.setRenderer(this);
        surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        service = new NodeService(this);

        lastFrameTime = System.currentTimeMillis();

        installRequested = false;
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (session == null) {
            Exception exception = null;
            String message = null;
            try {
                switch (ArCoreApk.getInstance().requestInstall(this, !installRequested)) {
                    case INSTALL_REQUESTED:
                        installRequested = true;
                        return;
                    case INSTALLED:
                        break;
                }

                // ARCore requires camera permissions to operate. If we did not yet obtain runtime
                // permission on Android M and above, now is a good time to ask the user for it.
                if (!CameraPermissionHelper.hasCameraPermission(this)) {
                    CameraPermissionHelper.requestCameraPermission(this);
                    return;
                }

                // Create the session.
                session = new Session(/* context= */ this);

            } catch (UnavailableArcoreNotInstalledException
                    | UnavailableUserDeclinedInstallationException e) {
                message = "Please install ARCore";
                exception = e;
            } catch (UnavailableApkTooOldException e) {
                message = "Please update ARCore";
                exception = e;
            } catch (UnavailableSdkTooOldException e) {
                message = "Please update this app";
                exception = e;
            } catch (UnavailableDeviceNotCompatibleException e) {
                message = "This device does not support AR";
                exception = e;
            } catch (Exception e) {
                message = "Failed to create AR session";
                exception = e;
            }

            if (message != null) {
                messageSnackbarHelper.showError(this, message);
                Log.e(TAG, "Exception creating session", exception);
                return;
            }
        }

        // Note that order matters - see the note in onPause(), the reverse applies here.
        try {
            session.resume();
        } catch (CameraNotAvailableException e) {
            // In some cases (such as another camera app launching) the camera may be given to
            // a different app instead. Handle this properly by showing a message and recreate the
            // session at the next iteration.
            messageSnackbarHelper.showError(this, "Camera not available. Please restart the app.");
            session = null;
            return;
        }

        surfaceView.onResume();
        displayRotationHelper.onResume();

        messageSnackbarHelper.showMessage(this, "Searching for surfaces...");
    }

    @Override
    public void onPause() {
        super.onPause();
        if (session != null) {
            // Note that the order matters - GLSurfaceView is paused first so that it does not try
            // to query the session. If Session is paused before GLSurfaceView, GLSurfaceView may
            // still call session.update() and get a SessionPausedException.
            displayRotationHelper.onPause();
            surfaceView.onPause();
            session.pause();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            Toast.makeText(this, "Camera permission is needed to run this application", Toast.LENGTH_LONG)
                    .show();
            if (!CameraPermissionHelper.shouldShowRequestPermissionRationale(this)) {
                // Permission denied with checking "Do not ask again".
                CameraPermissionHelper.launchPermissionSettings(this);
            }
            finish();
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        FullScreenHelper.setFullScreenOnWindowFocusChanged(this, hasFocus);
    }
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        int[] textures = new int[1];
        GLES30.glGenTextures(1, textures, 0);
        mTextureId = textures[0];
        int textureTarget = GLES11Ext.GL_TEXTURE_EXTERNAL_OES;
        GLES30.glBindTexture(textureTarget, mTextureId);
        GLES30.glTexParameteri(textureTarget, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameteri(textureTarget, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameteri(textureTarget, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_NEAREST);
        GLES30.glTexParameteri(textureTarget, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_NEAREST);
        GLES30.glBindTexture(textureTarget, 0);

        session.setCameraTextureName(mTextureId);


        //GLES20.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

//        // Prepare the rendering objects. This involves reading shaders, so may throw an IOException.
//        try {
//            // Create the texture and pass it to ARCore session to be filled during update().
//            backgroundRenderer.createOnGlThread(/*context=*/ this);
//            planeRenderer.createOnGlThread(/*context=*/ this, "models/trigrid.png");
//            pointCloudRenderer.createOnGlThread(/*context=*/ this);
//
//            virtualObject.createOnGlThread(/*context=*/ this, "models/andy.obj", "models/andy.png");
//            virtualObject.setMaterialProperties(0.0f, 2.0f, 0.5f, 6.0f);
//
//            virtualObjectShadow.createOnGlThread(
//                    /*context=*/ this, "models/andy_shadow.obj", "models/andy_shadow.png");
//            virtualObjectShadow.setBlendMode(BlendMode.Shadow);
//            virtualObjectShadow.setMaterialProperties(1.0f, 0.0f, 0.0f, 1.0f);
//
//        } catch (IOException e) {
//            Log.e(TAG, "Failed to read an asset file", e);
//        }
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        displayRotationHelper.onSurfaceChanged(width, height);
        //GLES20.glViewport(0, 0, width, height);

        if (!serviceInitialized) {
            service.init("http://192.168.0.13:8000/?e=hmd", "ar", mTextureId);
            // service.onSurfaceCreated();
            serviceInitialized = true;
        }
        service.onResize(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        // Clear screen to notify driver it should not load any pixels from previous frame.
        //GLES20.glClear(GLES30.GL_COLOR_BUFFER_BIT | GLES30.GL_DEPTH_BUFFER_BIT);

        if (session == null) {
            return;
        }
        // Notify ARCore session that the view size changed so that the perspective matrix and
        // the video background can be properly adjusted.
        displayRotationHelper.updateSessionIfNeeded(session);

        try {
            //session.setCameraTextureName(backgroundRenderer.getTextureId());

            // Obtain the current frame from ARSession. When the configuration is set to
            // UpdateMode.BLOCKING (it is by default), this will throttle the rendering to the
            // camera framerate.
            Frame frame = session.update();
            Camera camera = frame.getCamera();

            // Handle one tap per frame.
            handleTap(frame, camera);

            // Draw background.
            //backgroundRenderer.draw(frame);

//            // If not tracking, don't draw 3d objects.
//            if (camera.getTrackingState() == TrackingState.PAUSED) {
//                return;
//            }

            // Get projection matrix.
            float[] projmtx = new float[16];
            camera.getProjectionMatrix(projmtx, 0, 0.1f, 100.0f);

            // Get camera matrix and draw.
            float[] viewmtx = new float[16];
            camera.getViewMatrix(viewmtx, 0);

            float[] centerArray = new float[3];
            if (mAnchor == null) {
                Collection<Plane> allPlanes = session.getAllTrackables(Plane.class);
                if (allPlanes.size() > 0) {
                    Plane plane = allPlanes.iterator().next();
                    mAnchor = plane.createAnchor(plane.getCenterPose());
                }
            }
            if (mAnchor != null) {
                Pose center = mAnchor.getPose();
                centerArray[0] = center.tx();
                centerArray[1] = center.ty();
                centerArray[2] = center.tz();
            }

            service.onDrawFrame(viewmtx, projmtx, centerArray);

            long now = System.currentTimeMillis();
            int timeout = (int)Math.min(Math.max(FRAME_TIME_MAX - (now - lastFrameTime), FRAME_TIME_MIN), FRAME_TIME_MAX);
            lastFrameTime = now;
            service.tick(timeout);

//            // Compute lighting from average intensity of the image.
//            // The first three components are color scaling factors.
//            // The last one is the average pixel intensity in gamma space.
//            final float[] colorCorrectionRgba = new float[4];
//            frame.getLightEstimate().getColorCorrection(colorCorrectionRgba, 0);

//            // Visualize tracked points.
//            PointCloud pointCloud = frame.acquirePointCloud();
//            pointCloudRenderer.update(pointCloud);
//            pointCloudRenderer.draw(viewmtx, projmtx);
//
//            // Application is responsible for releasing the point cloud resources after
//            // using it.
//            pointCloud.release();
//
//            // Check if we detected at least one plane. If so, hide the loading message.
//            if (messageSnackbarHelper.isShowing()) {
//                for (Plane plane : session.getAllTrackables(Plane.class)) {
//                    if (plane.getTrackingState() == TrackingState.TRACKING) {
//                        messageSnackbarHelper.hide(this);
//                        break;
//                    }
//                }
//            }
//
//            // Visualize planes.
//            planeRenderer.drawPlanes(
//                    session.getAllTrackables(Plane.class), camera.getDisplayOrientedPose(), projmtx);
//
//            // Visualize anchors created by touch.
//            float scaleFactor = 1.0f;
//            for (ColoredAnchor coloredAnchor : anchors) {
//                if (coloredAnchor.anchor.getTrackingState() != TrackingState.TRACKING) {
//                    continue;
//                }
//                // Get the current pose of an Anchor in world space. The Anchor pose is updated
//                // during calls to session.update() as ARCore refines its estimate of the world.
//                coloredAnchor.anchor.getPose().toMatrix(anchorMatrix, 0);
//
//                // Update and draw the model and its shadow.
//                virtualObject.updateModelMatrix(anchorMatrix, scaleFactor);
//                virtualObjectShadow.updateModelMatrix(anchorMatrix, scaleFactor);
//                virtualObject.draw(viewmtx, projmtx, colorCorrectionRgba, coloredAnchor.color);
//                virtualObjectShadow.draw(viewmtx, projmtx, colorCorrectionRgba, coloredAnchor.color);
//            }

        } catch (Throwable t) {
            // Avoid crashing the application due to unhandled exceptions.
            Log.e(TAG, "Exception on the OpenGL thread", t);
        }
    }

    // Handle only one tap per frame, as taps are usually low frequency compared to frame rate.
    private void handleTap(Frame frame, Camera camera) {
        MotionEvent tap = tapHelper.poll();
        if (tap != null && camera.getTrackingState() == TrackingState.TRACKING) {
            for (HitResult hit : frame.hitTest(tap)) {
                // Check if any plane was hit, and if it was hit inside the plane polygon
                Trackable trackable = hit.getTrackable();
                // Creates an anchor if a plane or an oriented point was hit.
                if ((trackable instanceof Plane
                        && ((Plane) trackable).isPoseInPolygon(hit.getHitPose())
                        && (PlaneRenderer.calculateDistanceToPlane(hit.getHitPose(), camera.getPose()) > 0))
                        || (trackable instanceof Point
                        && ((Point) trackable).getOrientationMode()
                        == OrientationMode.ESTIMATED_SURFACE_NORMAL)) {
                    // Hits are sorted by depth. Consider only closest hit on a plane or oriented point.
                    // Cap the number of objects created. This avoids overloading both the
                    // rendering system and ARCore.
                    if (mAnchor != null) {
                        mAnchor.detach();
                    }

//                    // Assign a color to the object for rendering based on the trackable type
//                    // this anchor attached to. For AR_TRACKABLE_POINT, it's blue color, and
//                    // for AR_TRACKABLE_PLANE, it's green color.
//                    float[] objColor;
//                    if (trackable instanceof Point) {
//                        objColor = new float[] {66.0f, 133.0f, 244.0f, 255.0f};
//                    } else if (trackable instanceof Plane) {
//                        objColor = new float[] {139.0f, 195.0f, 74.0f, 255.0f};
//                    } else {
//                        objColor = DEFAULT_COLOR;
//                    }

                    // Adding an Anchor tells ARCore that it should track this position in
                    // space. This anchor is created on the Plane to place the 3D model
                    // in the correct position relative both to the world and to the plane.
                    mAnchor = hit.createAnchor();
                    break;
                }
            }
        }
    }
}