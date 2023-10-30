#pragma once

#include "Mode.h"
#include "Math/ellipse_math.h"

enum CalibrationModeState
{
	WAITING = 1,
	ACCELEROMETER = 2,
	GYROSCOPE = 4,
	MAGNETOMETER = 8,
	READY_TO_RECORD = 16,
	RECORDING_DATA = 32,
	STOP_RECORD = 64,
	UPDATE_CAL_NUMBERS = 128,
	ODR_ERROR = 256
};

class CalibrationMode : public Mode
{
public:
	CalibrationMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void update() override;
	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) override;

	std::wstring getCurrentlySelectedDevice() { return m_currentlySelectedDeviceAddress; }

	virtual uint32_t handleUIElementStateChange(int i) override;

	//Methods called from the Mode Screen class
	void startDataCapture();
	void stopDataCapture();
	void updateComplete();
	virtual void addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples) override;

	std::pair<float*, float**> getCalibrationResults();

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
	void initializeCalibrationVariables();

	//Methods for maneuvering through the calibrations
	void accelerometerCalibration();
	void gyroscopeCalibration();
	void magnetometerCalibration();

	void calculateCalNumbers();

	void advanceToNextStage();
	void prepareRecording();
	void displayGraph();

	void updateEllipsePoints(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z, Eigen::MatrixXf& b);

	float integrateData(float p1, float p2, float t)
	{
		return t * ((p1 + p2) / 2);
	}
	void invertAccMatrix();

	std::wstring m_currentlySelectedDeviceAddress = L"";
	int m_currentStage; //keeps track of the current stage of the calibration
	bool m_stageSet; //a variable used to make sure we don't keep reloading resources when we're on the same calibration stage
	bool m_useCalibratedData; //use calibrated data to benchmark current calibration numbers

	int raw_acceleration = 3, raw_rotation = 4, raw_magnetic = 5; //these variables match the DataType enumclass. Trying to use that enumclass here causes a circular reference

	//timing variables
	std::vector<float> data_time_stamps;
	float m_sensorODR;
	long long data_timer_duration, data_timer_elapsed;
	std::chrono::steady_clock::time_point data_timer;

	//data variables
	std::vector<DirectX::XMFLOAT2> m_graphDataX, m_graphDataY, m_graphDataZ;
	float m_timeStamp;
	float acc_cal[3][6]; //needed to isolate data from all six portions of the acc. tumble calibration: x1, x2, x3, x4, x5, x6, y1, y2... z6
	std::vector<float> mx, my, mz; //holds calibrated magnetometer data
	int avg_count; //used for averaging accelerometer data from tumble test

	//calibration variables
	bool accept_cal;

	float acc_off[3] = { 0 }; //acceleration offset values
	float acc_gain_x[3] = { 0 }; //acceleration axis and cross axis gain values
	float acc_gain_y[3] = { 0 }; //acceleration axis and cross axis gain values
	float acc_gain_z[3] = { 0 }; //acceleration axis and cross axis gain values
	float* acc_gain[3] = { acc_gain_x, acc_gain_y, acc_gain_z }; //acceleration axis and cross axis gain values

	float gyr_off[3] = { 0 };
	float gyr_gain_x[3] = { 0 };
	float gyr_gain_y[3] = { 0 };
	float gyr_gain_z[3] = { 0 };
	float* gyr_gain[3] = {gyr_gain_x, gyr_gain_y, gyr_gain_z};

	float mag_off[3] = { 0 };
	float mag_gain_x[3] = { 0 };
	float mag_gain_y[3] = { 0 };
	float mag_gain_z[3] = { 0 };
	float* mag_gain[3] = { mag_gain_x, mag_gain_y, mag_gain_z };
};