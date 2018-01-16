// Copyright (c) 2014 by B.W. van Schooten, info@borisvanschooten.nl
package com.mafintosh.nodeonandroid;

public class GlesJSLib {

  // GL
	public static native void onSurfaceCreated();
	public static native void onSurfaceChanged(int width, int height);

  // VR
  public static native void onNewFrame(float[] headViewMatrix, float[] headQuaternion);
	public static native void onDrawEye(float[] eyeViewMatrix, float[] eyePerspectiveMatrix);

  // AR
	public static native void onDrawFrame(float[] viewMatrix, float[] projectionMatrix);

	public static native void onTouchEvent(int id,double x,double y,
	boolean press,boolean release);

	public static native void onMultitouchCoordinates(int id,double x,double y);

	public static native void onControllerEvent(int player,boolean active,
	boolean [] buttons, float [] axes);
}
