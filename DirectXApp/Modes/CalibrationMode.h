#pragma once

#include "Mode.h"

enum CalibrationModeState
{
	WAITING = 1,
	ACCELEROMETER = 2,
	GYROSCOPE = 4,
	MAGNETOMETER = 8,
	READY_TO_RECORD = 16,
	RECORDING_DATA = 32,
	STOP_RECORD = 64
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

	void startDataCapture();
	void stopDataCapture();

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);

	//Methods for maneuvering through the calibrations
	void accelerometerCalibration();
	void gyroscopeCalibration();
	void magnetometerCalibration();

	void advanceToNextStage();

	std::wstring m_currentlySelectedDeviceAddress = L"";
	int m_currentStage; //keeps track of the current stage of the calibration
	bool m_stageSet; //a variable used to make sure we don't keep reloading resources when we're on the same calibration stage

	//timing variables
	long long data_timer_duration, data_timer_elapsed;
	std::chrono::steady_clock::time_point data_timer;
};