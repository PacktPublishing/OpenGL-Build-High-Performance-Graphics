/*
 * Sensor.h
 */

#ifndef SENSOR_H_
#define SENSOR_H_
#include <stdlib.h>
#include <jni.h>
#include <GLES3/gl3.h>
#include <math.h>


class Sensor {

public:
	Sensor();
	Sensor(unsigned int size);
	virtual ~Sensor();

	//can resize the buffer size dynamically by calling this function
	void init(unsigned int size);
	//append the new data to the buffer
	void appendAccelData(GLfloat x, GLfloat y, GLfloat z);
	void appendGyroData(GLfloat x, GLfloat y, GLfloat z);
	void appendMagData(GLfloat x, GLfloat y, GLfloat z);

	//get sensor data buffer
	GLfloat *getAccelDataPtr(int channel);
	GLfloat *getGyroDataPtr(int channel);
	GLfloat *getMagDataPtr(int channel);
	GLfloat *getAxisPtr();

	//the auto rescale factors based on max and min
	GLfloat getAccScale();
	GLfloat getGyroScale();
	GLfloat getMagScale();

	//get the buffer size
	unsigned int getBufferSize();

private:
	unsigned int buffer_size;

	GLfloat **accel_data;
	GLfloat **gyro_data;
	GLfloat **mag_data;
	GLfloat *x_axis;

	GLfloat abs_max_acc;
	GLfloat abs_max_mag;
	GLfloat abs_max_gyro;

	void createBuffers(unsigned int size);
	void free_all();

	void findAbsMax(GLfloat *src, GLfloat *max);
	void appendData(GLfloat *src, GLfloat data);
	void setNormalizedAxis(GLfloat *data, unsigned int size, float min, float max);

};


#endif /* SENSOR_H_ */
