package com.android.gl3jni;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;


/**
 * Main application for Android, with Sensor inputs
 */
public class GL3JNIActivity extends Activity implements SensorEventListener{
    GL3JNIView mView;
    private SensorManager mSensorManager;
    private Sensor mAccelerometer;
    private Sensor mGyro;
    private Sensor mMag;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        //lock the screen orientation for this demo
        //otherwise, the canvas will rotate
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        
        //create the Sensor Manager to handle all sensor information
        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        
        //Get sensor objects for accelerometer, gyroscope, and digital compass
        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        mGyro = mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        mMag = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        
        mView = new GL3JNIView(getApplication());
        setContentView(mView);
    }

    /**
     * We must unregister the sensor listener; otherwise, the program
     * will constantly draw sensor data and quickly drain the battery of the device
     */
    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
        //unregister accelerometer and other sensors
        mSensorManager.unregisterListener(this, mAccelerometer);
        mSensorManager.unregisterListener(this, mGyro);
        mSensorManager.unregisterListener(this, mMag);
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
        //register and activate the sensors. Start streaming data and handle with
        //callback functions
        mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_GAME);
        mSensorManager.registerListener(this, mGyro, SensorManager.SENSOR_DELAY_GAME);
        mSensorManager.registerListener(this, mMag, SensorManager.SENSOR_DELAY_GAME);
    }

    //sensor events
	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		//ignore for now
	}

	/**
	 * Handle the sensor events when new data is available. SensorEvent
	 * holds all the information about the sensor (type, time-stamp, accuracy...)
	 * http://developer.android.com/reference/android/hardware/SensorEvent.html#values
	 */
	@Override
	public void onSensorChanged(SensorEvent event) {
		//handle the accelerometer data
		//All values are in SI units (m/s^2)
		if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
			float ax, ay, az;
			ax = event.values[0];
			ay = event.values[1];
			az = event.values[2];
			GL3JNILib.addAccelData(ax, ay, az);
	    }
		//All values are in radians/second and measure the rate of 
		//rotation around the device's local X, Y and Z axes
		if (event.sensor.getType() == Sensor.TYPE_GYROSCOPE) {
			float gx, gy, gz;
			//angular speed
			gx = event.values[0];
			gy = event.values[1];
			gz = event.values[2];
			GL3JNILib.addGyroData(gx, gy, gz);
	    }
		//All values are in micro-Tesla (uT) and measure the ambient magnetic 
		//field in the X, Y and Z axes.
		if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
			float mx, my, mz;
			mx = event.values[0]; 
			my = event.values[1];
			mz = event.values[2];
			GL3JNILib.addMagData(mx, my, mz);
	    }

	}
}
