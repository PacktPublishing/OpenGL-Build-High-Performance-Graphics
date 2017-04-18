package com.android.gl3jni;

// Wrapper for native library

/**
 * Linking the native library calls to Java
 */
public class GL3JNILib {
    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native void init(int width, int height);
     public static native void step();

     //pass the image to JNI C++ side
     public static native void setImage(long imageRGBA);
     
     //pass the device rotation angles and the scaling factor to the OpenGL ES 3.0
     public static native void resetRotDataOffset();
     public static native void setRotMatrix(float[] rotMatrix);
     public static native void setScale(float scale);

     public static native void toggleFeatures();
     public static native void setDxDy(float dx, float dy);
}
