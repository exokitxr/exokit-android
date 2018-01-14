// Copyright (c) 2014 by B.W. van Schooten, info@borisvanschooten.nl
package com.mafintosh.nodeonandroid;

public class GlesJSLib {

	static {
		System.loadLibrary("node");
		System.loadLibrary("nodebinding");
	}

	public static native void onSurfaceCreated();
	public static native void onSurfaceChanged(int width, int height);
	public static native void onDrawFrame();

	public static native void onTouchEvent(int id,double x,double y,
	boolean press,boolean release);

	public static native void onMultitouchCoordinates(int id,double x,double y);

	public static native void onControllerEvent(int player,boolean active,
	boolean [] buttons, float [] axes);
}
