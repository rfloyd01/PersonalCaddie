#pragma once

#include "Mode.h"

#include "Math/SensorFusion/FusionAhrs.h"
#include "Math/SensorFusion/FusionOffset.h"

class FreeSwingMode : public Mode
{
public:
	FreeSwingMode();

	virtual uint32_t initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState = 0) override;
	virtual void uninitializeMode() override;

	virtual void pc_ModeChange(PersonalCaddiePowerMode newMode) override;

	virtual void update() override;
	virtual void handleKeyPress(winrt::Windows::System::VirtualKey pressedKey) override;

	virtual void getIMUHeadingOffset(glm::quat heading) override;
	virtual void addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t) override;

	void setCurrentHeadingOffset();

private:
	void initializeTextOverlay();
	void loadModel();

	void convergenceCheck();

	std::chrono::steady_clock::time_point data_start_timer;

	volatile int m_currentQuaternion;
	volatile bool m_update_in_process;
	DirectX::XMVECTOR m_renderQuaternion; //the current quaternion to be applied on screen
	std::vector<glm::quat> m_quaternions;
	glm::quat m_headingOffset = { 1.0f, 0.0f, 0.0f, 0.0f };
	std::vector<float> m_timeStamps; //helps figure out which quaternion to actually render (depends on screen refresh rate)

	bool m_converged;
	std::vector<glm::quat> m_convergenceQuaternions;

	//Array used to swap real world coordinates to DirectX coordinates
	int computer_axis_from_sensor_axis[3] = {1, 2, 0};
};