package com.android.gl3jni;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.content.Context;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.CvType;
import org.opencv.core.Mat;

import android.widget.RelativeLayout;
import android.view.SurfaceView;

/**
 * Main application for Android, with Sensor inputs
 */
public class GL3OpenCVDemo extends Activity implements SensorEventListener,
		CvCameraViewListener2 {
	private GL3JNIView mView = null;
	private Context context;
	private SensorManager mSensorManager;
	private Sensor mRotate;
	private float[] mRotationMatrix = new float[16];
	private float[] orientationVals = new float[3];
	private boolean gl3_loaded = false;
	private CameraBridgeViewBase mOpenCvCameraView;
	private RelativeLayout l_layout;

	// Callback function for OpenCV
	private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
		@Override
		public void onManagerConnected(int status) {
			switch (status) {
			case LoaderCallbackInterface.SUCCESS: {
				Log.i("OpenCVDemo", "OpenCV loaded successfully");

				// load the library *AFTER* we have OpenCV lib ready!
				System.loadLibrary("gl3jni");
				gl3_loaded = true;

				// load the view as we have all JNI loaded
				mView = new GL3JNIView(getApplication());
				l_layout.addView(mView);
				setContentView(l_layout);

				// enable the camera, and push the images to the OpenGL layer
				mOpenCvCameraView.enableView();
			}
				break;
			default: {
				super.onManagerConnected(status);
			}
				break;
			}
		}
	};

	public void onCameraViewStarted(int width, int height) {
	}

	public void onCameraViewStopped() {

	}

	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		// Log.i("OpenCVDemo", "Got Frame\n");
		// should call the opengl code
		Mat input = inputFrame.rgba();
		if (gl3_loaded) {
			GL3JNILib.setImage(input.nativeObj);
		}
		// don't show on the java side
		return null;
	}

	@Override
	protected void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		// lock the screen orientation for this demo
		// otherwise the canvas will rotate
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		mSensorManager = (SensorManager) getSystemService(SENSOR_SERVICE);

		// TYPE_ROTATION_VECTOR for device orientation
		mRotate = mSensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);

		// set up the Java Camera with OpenCV
		setContentView(R.layout.ar);
		l_layout = (RelativeLayout) findViewById(R.id.linearLayoutRest);
		mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.opencv_camera_surface_view);
		mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
		mOpenCvCameraView.setMaxFrameSize(1280, 720); // cap it at 720 for performance
		mOpenCvCameraView.setCvCameraViewListener(this);
		mOpenCvCameraView.disableView();
	}

	// We must unregister the sensor listener or otherwise the program
	// will constantly draw sensor data and quickly draw the battery of the
	// device in hours
	@Override
	protected void onPause() {
		super.onPause();
		mSensorManager.unregisterListener(this);
		// stop the camera
		if (mView != null) {
			mView.onPause();
		}
		if (mOpenCvCameraView != null)
			mOpenCvCameraView.disableView();
		gl3_loaded = false;
	}

	// register the sensor, and set the refresh rate to SENSOR_DELAY_GAME
	// for fast response inputs (high sample rate update).
	@Override
	protected void onResume() {
		super.onResume();
		// Initialize OpenCV with async call (Note: only load OpenCV libs
		// after this call.)
		OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this,
				mLoaderCallback);
		mSensorManager.registerListener(this, mRotate,
				SensorManager.SENSOR_DELAY_GAME);
		if (mView != null) {
			mView.onResume();
		}
	}

	// sensor events
	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {

	}

	// handle the device orientation data, save the event data as rotation
	// matrix and obtain the orientation (in angles) and pass
    // it to the native code OpenGL renderer
	@Override
	public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR) {
			SensorManager.getRotationMatrixFromVector(mRotationMatrix,
					event.values);
			// remap the coodinate system to align with the OpenGL's camera
			SensorManager.remapCoordinateSystem(mRotationMatrix,
					SensorManager.AXIS_Y, SensorManager.AXIS_MINUS_X,
					mRotationMatrix);
			if (gl3_loaded)
				GL3JNILib.setRotMatrix(mRotationMatrix);
		}
	}
}

