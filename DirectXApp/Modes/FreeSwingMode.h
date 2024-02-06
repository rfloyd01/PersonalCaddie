#pragma once

#include "Mode.h"

#include "Math/SensorFusion/FusionAhrs.h"
#include "Math/SensorFusion/FusionOffset.h"
//#include "Math/quaternion_functions.h"
#include "Golf/SwingPhaseDetection.h"

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
	virtual void addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples) override;

	void setCurrentHeadingOffset();

private:
	void initializeTextOverlay();
	void loadModel();

	void convergenceCheck();

	void swingUpdate();

	std::chrono::steady_clock::time_point data_start_timer;

	volatile int m_currentQuaternion;
	volatile bool m_update_in_process;
	DirectX::XMVECTOR m_renderQuaternion; //the current quaternion to be applied on screen
	std::vector<glm::quat> m_quaternions;
	std::vector<ClubEulerAngles> m_eulerAngles; //holds euler angles coming from the Personal Caddie
	std::vector<std::pair<float, float> > m_angularVelocities; //each pair holds the pitch and yaw angular velocities as read from the sensor
	glm::quat m_headingOffset = { 1.0f, 0.0f, 0.0f, 0.0f };
	std::vector<float> m_timeStamps; //helps figure out which quaternion to actually render (depends on screen refresh rate)
	float m_sensorODR = 0.0f;

	bool m_converged;
	std::vector<glm::quat> m_convergenceQuaternions;

	//Swing phase variables
	SwingPhase m_swing_phase;
	ClubEulerAngles m_current_club_angles, m_initial_club_angles;
	std::chrono::time_point<std::chrono::steady_clock> m_swing_start_time;
	std::vector<float> m_ball_location;
	int m_backswing_point; //keeps track of which point is being looked at for the data average
	float m_previous_pitch_average, m_current_pitch_average;
	float m_previous_yaw_average, m_current_yaw_average;
	std::vector<DirectX::XMFLOAT2> m_swingPath; //Tracks the club path through the impact zone
	volatile bool m_newQuaternions = false;

	//Array used to swap real world coordinates to DirectX coordinates
	int computer_axis_from_sensor_axis[3] = {1, 2, 0};
};