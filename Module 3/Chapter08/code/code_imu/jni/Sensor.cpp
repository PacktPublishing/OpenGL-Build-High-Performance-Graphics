/*
 * Sensor.cpp
 */

#include "Sensor.h"

/**
 * Constructor
 */
Sensor::Sensor() {
	//use default size
	init(256);
}

/**
 * Initialize with different buffer size
 */
Sensor::Sensor(unsigned int size) {
	init(size);
}

/**
 * Destructor
 */
Sensor::~Sensor() {
	//free up all memory allocated
	free_all();
}

/**
 * Initialize all variables and buffers
 */
void Sensor::init(unsigned int size){
	buffer_size = size;
	//delete the old memory if already exist
	free_all();
	//calloc the memory for the buffer
	createBuffers(size);
	setNormalizedAxis(x_axis,size, -1.0f, 1.0f);
	abs_max_acc = 0;
	abs_max_gyro = 0;
	abs_max_mag = 0;
}


/**
 * Allocate memory for all sensor data buffers
 */
void Sensor::createBuffers(unsigned int buffer_size){
	accel_data = (GLfloat**)malloc(3*sizeof(GLfloat*));
	gyro_data = (GLfloat**)malloc(3*sizeof(GLfloat*));
	mag_data = (GLfloat**)malloc(3*sizeof(GLfloat*));

	//3 channels for accelerometer
	accel_data[0] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));
	accel_data[1] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));
	accel_data[2] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));

	//3 channels for gyroscope
	gyro_data[0] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));
	gyro_data[1] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));
	gyro_data[2] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));

	//3 channels for digital compass
	mag_data[0] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));
	mag_data[1] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));
	mag_data[2] = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));

	//x-axis precomputed
	x_axis = (GLfloat*)calloc(buffer_size,sizeof(GLfloat));;
}

/**
 * Deallocate all memory
 */
void Sensor::free_all(){
	if(accel_data){
		free(accel_data[0]);
		free(accel_data[1]);
		free(accel_data[2]);
		free(accel_data);
	}
	if(gyro_data){
		free(gyro_data[0]);
		free(gyro_data[1]);
		free(gyro_data[2]);
		free(gyro_data);
	}
	if(mag_data){
		free(mag_data[0]);
		free(mag_data[1]);
		free(mag_data[2]);
		free(mag_data);
	}
	if(x_axis){
		free(x_axis);
	}
}

/**
 * Return the global max for the acceleration data buffer (for rescaling and fitting purpose)
 */
GLfloat Sensor::getAccScale() {
	return abs_max_acc;
}

/**
 * Return the global max for the gyroscope data buffer (for rescaling and fitting purpose)
 */
GLfloat Sensor::getGyroScale() {
	return abs_max_gyro;
}

/**
 * Return the global max for the magnetic field data buffer (for rescaling and fitting purpose)
 */
GLfloat Sensor::getMagScale() {
	return abs_max_mag;
}
/**
 * Find the absolute maximum from the buffer
 */
void Sensor::findAbsMax(GLfloat *src, GLfloat *max){
	int i=0;
	for(i=0; i<buffer_size; i++){
		if(*max < fabs(src[i])){
			*max= fabs(src[i]);
		}
	}
}
/**
 * Pre-compute the x-axis for the plot
 */
void Sensor::setNormalizedAxis(GLfloat *data, unsigned int size, float min, float max){
	float step_size = (max - min)/(float)size;
	for(int i=0; i<size; i++){
		data[i]=min+step_size*i;
	}
}
/**
 * Append acceleration data to the buffer
 */
void Sensor::appendAccelData(GLfloat x, GLfloat y, GLfloat z){
	abs_max_acc = 0;
	float data[3] = {x, y, z};
	for(int i=0; i<3; i++){
		appendData(accel_data[i], data[i]);
		findAbsMax(accel_data[i], &abs_max_acc);
	}
}

/**
 * Append the gyroscope data to the buffer
 */
void Sensor::appendGyroData(GLfloat x, GLfloat y, GLfloat z){
	abs_max_gyro = 0;
	float data[3] = {x, y, z};
	for(int i=0; i<3; i++){
		appendData(gyro_data[i], data[i]);
		findAbsMax(gyro_data[i], &abs_max_gyro);
	}
}

/**
 * Append the magnetic field data to the buffer
 */
void Sensor::appendMagData(GLfloat x, GLfloat y, GLfloat z){
	abs_max_mag = 0;
	float data[3] = {x, y, z};
	for(int i=0; i<3; i++){
		appendData(mag_data[i], data[i]);
		findAbsMax(mag_data[i], &abs_max_mag);
	}
}

/**
 * Append Data to the end of the buffer
 */
void Sensor::appendData(GLfloat *src, GLfloat data){
	//shift the data by one
	int i;
	for(i=0; i<buffer_size-1; i++){
		src[i]=src[i+1];
	}
	//set the last element with the new data
	src[buffer_size-1]=data;
}

/**
 *Get the acceleration data buffer
 */
GLfloat* Sensor::getAccelDataPtr(int channel) {
	return accel_data[channel];
}

/**
 * Get the Gryoscope data buffer
 */
GLfloat* Sensor::getGyroDataPtr(int channel) {
	return gyro_data[channel];
}

/**
 * Get the Magnetic field data buffer
 */
GLfloat* Sensor::getMagDataPtr(int channel) {
	return mag_data[channel];
}

/**
 * Return the axis
 */
GLfloat* Sensor::getAxisPtr() {
	return x_axis;
}

/**
 * Return the buffer size
 */
unsigned int Sensor::getBufferSize() {
	return buffer_size;
}


//end of file

