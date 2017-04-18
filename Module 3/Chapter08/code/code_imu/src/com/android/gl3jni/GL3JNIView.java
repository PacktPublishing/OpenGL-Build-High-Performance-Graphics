package com.android.gl3jni;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * A simple application that uses OpenGL ES3 and GLSurface
 */
class GL3JNIView extends GLSurfaceView {
    public GL3JNIView(Context context) {
        super(context);
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil
        setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        setEGLContextClientVersion(3);
        getHolder().setFormat(PixelFormat.RGBA_8888);
        setRenderer(new Renderer());
    }
    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
        	GL3JNILib.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
        	GL3JNILib.init(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        	//do nothing
        }
    }
}
