#pragma once

#include "Mode.h"

#include "Math/SensorFusion/FusionAhrs.h"
#include "Math/SensorFusion/FusionOffset.h"

class MadgwickTestMode : public Mode
{
public:
	MadgwickTestMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void pc_ModeChange(PersonalCaddiePowerMode newMode) override;

	virtual void update() override;
	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

	virtual void getIMUHeadingOffset(glm::quat heading) override;
	virtual void addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t) override;
	virtual void addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples) override;

	void toggleDisplayData();
	void switchDisplayDataType(int n);

	void setCurrentHeadingOffset();

	void toggleFilter() { m_useNewFilter = !m_useNewFilter; }

private:
	void initializeTextOverlay();
	void updateDisplayText();

	void loadModel();

	void convergenceCheck();

	float m_currentRotation;
	float m_currentDegree;

	std::chrono::steady_clock::time_point data_start_timer;

	volatile int m_currentQuaternion;
	volatile bool m_update_in_process;
	DirectX::XMVECTOR m_renderQuaternion; //the current quaternion to be applied on screen
	std::vector<glm::quat> m_quaternions;
	glm::quat m_headingOffset = { 1.0f, 0.0f, 0.0f, 0.0f };
	std::vector<float> m_timeStamps;

	bool m_converged;
	std::vector<glm::quat> m_convergenceQuaternions;
	bool m_linearAcc, m_velocity, m_location; //to match the variables in the Personal Caddie class

	//Variables for Displaying Live data from sensor
	bool m_show_live_data;
	std::vector<float> m_display_data[3];
	std::wstring m_display_data_type, m_display_data_units;
	int m_display_data_index;
	DataType m_current_data_type;

	//Array used to swap real world coordinates to DirectX coordinates
	int computer_axis_from_sensor_axis[3] = {1, 2, 0};

	//New Madgwick Filter Testing
	FusionOffset m_offset;
	FusionAhrs m_ahrs;
	std::vector<glm::quat> m_testQuaternions;
	bool m_useNewFilter;
};