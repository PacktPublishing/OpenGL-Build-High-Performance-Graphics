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
}
