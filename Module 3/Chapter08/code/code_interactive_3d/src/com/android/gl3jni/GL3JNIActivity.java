
package com.android.gl3jni;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;


/**
 * Main application for Android, with Sensor inputs
 */
public class GL3JNIActivity extends Activity implements SensorEventListener{
    GL3JNIView mView;
    private SensorManager mSensorManager;
    private Sensor mRotate; 
    private float[] mRotationMatrix=new float[16];
    private float[] orientationVals=new float[3];

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        //lock the screen orientation for this demo
        //otherwise the canvas will rotate
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        
        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        //TYPE_ROTATION_VECTOR for device orientation
        mRotate = mSensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
        
        mView = new GL3JNIView(getApplication());
        setContentView(mView);
    }

    /**
     * We must unregister the sensor listener or otherwise the program
     * will constantly draw sensor data and quickly drain the battery of the device
     */
    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
        mSensorManager.unregisterListener(this);
    }

    //register the sensor, and set the refresh rate to SENSOR_DELAY_GAME
    //for fast response inputs (high sample rate update). 
    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
        mSensorManager.registerListener(this, mRotate, SensorManager.SENSOR_DELAY_GAME);
    }

    //sensor events
	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		
	}
	//handle the device orientation data, save the event data as rotation matrix
	//and obtain the orientation (in angles) and pass to the native code OpenGL renderer
	@Override
	public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() == Sensor.TYPE_ROTATION_VECTOR){
	        SensorManager.getRotationMatrixFromVector(mRotationMatrix,event.values);
	        SensorManager.getOrientation(mRotationMatrix, orientationVals);
			GL3JNILib.addRotData(orientationVals[0],orientationVals[1],orientationVals[2]);
		}	
	}
}
