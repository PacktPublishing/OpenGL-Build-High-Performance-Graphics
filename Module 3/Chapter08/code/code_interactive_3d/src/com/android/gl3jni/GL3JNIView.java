package com.android.gl3jni;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/*
    HOW_IT_WORKS:
    The ScaleGestureDetector class detects scaling transformation gesture 
    (i.e., pinch with two fingers) using the MotionEvent class from the multi-touch screen.
    The algorithm returns the scale factor which can be redirected to the OpenGL pipeline to
    update the graphics in real-time. The SimpleOnScaleGestureListener class provides a
    callback function for the onScale event and we pass the scale factor (mScaleFactor) to the
    native code using the GL3JNILib.setScale call.
 */

class GL3JNIView extends GLSurfaceView {
	private ScaleGestureDetector mScaleDetector;
	private Renderer renderer;
	
    public GL3JNIView(Context context) {
        super(context);
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
        setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        setEGLContextClientVersion(3);
        getHolder().setFormat(PixelFormat.RGBA_8888);
        renderer = new Renderer();
        setRenderer(renderer);
        //handle gesture input
        mScaleDetector = new ScaleGestureDetector(context, new ScaleListener());
    }
    //Draw graphics using the OpenGL ES 3.0 native code 
    private class Renderer implements GLSurfaceView.Renderer {    	
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
    //pass MotionEvent to the gesture detector
    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        // Let the ScaleGestureDetector inspect all events.
        mScaleDetector.onTouchEvent(ev);
        return true;
    }
    //implement SimpleOnScaleGestureListener and handle the callbacks
	private class ScaleListener extends
			ScaleGestureDetector.SimpleOnScaleGestureListener {
		//default scale = 1.0
    	private float mScaleFactor = 1.f;
		@Override
		public boolean onScale(ScaleGestureDetector detector) {
			//scaling factor
			mScaleFactor *= detector.getScaleFactor();
			// Don't let the object get too small or too large.
			mScaleFactor = Math.max(0.1f, Math.min(mScaleFactor, 5.0f));
			invalidate();
        	GL3JNILib.setScale(mScaleFactor);
			return true;
		}
	}
}
