#pragma once

#include "Mode.h"

class MadgwickTestMode : public Mode
{
public:
	MadgwickTestMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual uint32_t handleUIElementStateChange(int i) override;

	virtual void update() override;
	virtual void addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t) override;
	virtual void addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples) override;

	void toggleDisplayData();
	void switchDisplayDataType(int n);

	glm::quat getCurrentHeadingOffset();
	void setHeadingOffset(glm::quat offset) { m_headingOffset = offset; }

private:
	void initializeTextOverlay(winrt::Windows::Foundation::Size windowSize);
	void updateDisplayText();

	float m_currentRotation;
	float m_currentDegree;

	std::chrono::steady_clock::time_point data_start_timer;

	volatile int m_currentQuaternion;
	volatile bool m_update_in_process;
	DirectX::XMVECTOR m_renderQuaternion; //the current quaternion to be applied on screen
	std::vector<glm::quat> m_quaternions;
	//glm::quat m_offsetQuaternion = { 0.9848f,  0.0f, 0.0f, -0.1736f };
	glm::quat m_headingOffset = { 1.0f, 0.0f, 0.0f, 0.0f };
	std::vector<float> m_timeStamps;

	//Variables for Displaying Live data from sensor
	bool m_show_live_data;
	std::vector<float> m_display_data[3];
	std::wstring m_display_data_type, m_display_data_units;
	int m_display_data_index;

	//Array used to swap real world coordinates to DirectX coordinates
	int computer_axis_from_sensor_axis[3] = {1, 2, 0};

};