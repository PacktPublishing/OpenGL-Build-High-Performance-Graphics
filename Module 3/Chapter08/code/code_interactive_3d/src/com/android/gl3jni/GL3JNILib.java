package com.android.gl3jni;

// Wrapper for native library

/**
 * Linking the native library calls to Java
 */
public class GL3JNILib {

     static {
         System.loadLibrary("gl3jni");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native void init(int width, int height);
     public static native void step();
     
     //pass the device rotation angles and the scaling factor to the OpenGL ES 3.0 side
     public static native void addRotData(float rx, float ry, float rz);
     public static native void setScale(float scale);

}
