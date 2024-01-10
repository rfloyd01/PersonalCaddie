#pragma once

#include "Mode.h"
#include "Math/ellipse_math.h"

enum class SensorCalibrationAction
{
	GET_SENSOR_CAL,
	SET_SENSOR_CAL,
	GET_SENSOR_AXIS_CAL,
	SET_SENSOR_AXIS_CAL
};

struct CalibrationRequest
{
	SensorCalibrationAction action;
	sensor_type_t sensor;
	std::pair<float*, float**> cal_numbers;
	std::pair<int*, int*> axis_numbers;
};

class CalibrationMode : public Mode
{
public:
	CalibrationMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void update() override;
	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

	virtual void handlePersonalCaddieConnectionEvent(bool connectionStatus) override;

	std::wstring getCurrentlySelectedDevice() { return m_currentlySelectedDeviceAddress; }

	//Methods called from the Mode Screen class
	virtual void addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples) override;
	virtual void getSensorAxisCalibrationNumbers(sensor_type_t sensor, std::pair<const int*, const int*> cal_numbers) override;

	std::pair<float*, float**> getCalibrationResults();
	std::vector<int> getNewAxesOrientations();

	virtual void pc_ModeChange(PersonalCaddiePowerMode newMode) override;
	virtual void ble_NotificationsChange(int state) override;

private:
	void initializeTextOverlay();
	void initializeCalibrationVariables();

	//Handler Methods
	virtual void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element) override;

	//Methods for maneuvering through the calibrations
	void accelerometerCalibration();
	void accelerometerAxisCalibration();
	void gyroscopeCalibration();
	void gyroscopeAxisCalibration();
	void magnetometerCalibration();
	void magnetometerAxisCalibration();

	void calculateCalNumbers();

	void initializeModel();
	void loadModeMainPage();

	void advanceToNextStage();

	//Methods for recording data
	void prepareRecording();
	void record();
	void stopRecording();
	
	void displayGraph();

	void accAverageData();
	void accAxisCalculate(int axis);
	void gyrAxisCalculate(int axis);
	void magAxisCalculate(int axis);
	std::wstring axisResultString(int* axis_swap, int* axis_polarity);

	void updateEllipsePoints(std::vector<float>& x, std::vector<float>& y, std::vector<float>& z, Eigen::MatrixXf& b);

	float integrateData(float p1, float p2, float t)
	{
		return t * ((p1 + p2) / 2);
	}
	void invertAccMatrix();

	std::wstring m_currentlySelectedDeviceAddress = L"";
	int m_currentStage; //keeps track of the current stage of the calibration
	int m_currentSensor; //keeps track of which sensor is currently being calibrated
	bool m_stageSet; //a variable used to make sure we don't keep reloading resources when we're on the same calibration stage
	bool m_useCalibratedData; //use calibrated data to benchmark current calibration numbers
	bool m_axisCalibration; //If this is set to true then we calibrate a sensor's axes instead of it's data
	bool m_currentlyRecording; //This bool will be true when we're actively reading data from one of the sensors

	int raw_acceleration = 3, raw_rotation = 4, raw_magnetic = 5; //these variables match the DataType enumclass. Trying to use that enumclass here causes a circular reference

	//timing variables
	std::vector<float> data_time_stamps;
	float m_sensorODR;
	long long data_timer_duration, data_timer_elapsed;
	std::chrono::steady_clock::time_point data_timer;

	//Rendering Variables
	glm::quat m_quaternion; //it's easier for me to do rotation calculations with GLM quaternions than DirectX Vectors
	DirectX::XMVECTOR m_renderQuaternion; //the current quaternion to be applied on screen
	int clockwise_rotation = 1;

	//data variables
	std::vector<DirectX::XMFLOAT2> m_graphDataX, m_graphDataY, m_graphDataZ;
	float m_timeStamp;
	float acc_cal[3][6]; //needed to isolate data from all six portions of the acc. tumble calibration: x1, x2, x3, x4, x5, x6, y1, y2... z6
	std::vector<float> mx, my, mz; //holds calibrated magnetometer data
	int avg_count; //used for averaging accelerometer data from tumble test

	//calibration variables
	bool accept_cal, unlimited_record;

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

	int *m_axis_swap, *m_axis_polarity; //variables used to point to the axis calibration data of the sensor currently being calibrated
	int acc_axis_swap[3] = { 0 };
	int acc_axis_polarity[3] = { 0 };
	int test_acc_axis_swap[3] = { 0 };
	int test_acc_axis_polarity[3] = { 0 };

	int gyr_axis_swap[3] = { 0 };
	int gyr_axis_polarity[3] = { 0 };
	int test_gyr_axis_swap[3] = { 0 };
	int test_gyr_axis_polarity[3] = { 0 };

	int mag_axis_swap[3] = { -1, -1, -1 };
	int mag_axis_polarity[3] = { 1, 1, 1 };
	int test_mag_axis_swap[3] = { -1, -1, -1 };
	int test_mag_axis_polarity[3] = { 1, 1, 1 };
};