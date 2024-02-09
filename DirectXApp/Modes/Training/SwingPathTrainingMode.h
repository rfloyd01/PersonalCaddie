#pragma once

#include "../Mode.h"

#include "Math/SensorFusion/FusionAhrs.h"
#include "Math/SensorFusion/FusionOffset.h"
#include "Golf/GolfSwing.h"

class SwingPathTrainingMode : public Mode, GolfSwing
{
public:
	SwingPathTrainingMode();

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
	void loadTrainingUI();
	void loadModel();

	void convergenceCheck();

	float calculateSwingSpeed();

	//Handler Methods
	virtual void uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element) override;

	//Overridden Golf Swing Methods
	virtual void impactAction() override;
	virtual void preAddressAction() override;
	virtual void preBackswingAction() override;
	virtual void preTransitionAction() override;
	virtual void preDownswingAction() override;
	virtual void preImpactAction() override;
	virtual void preFollowThroughAction() override;
	virtual void preSwingEndAction() override;

	//Training Module Specific Methods
	void switchMode();
	void levelUp();
	void updateGameText();
	void createGoalAngle();

	std::chrono::steady_clock::time_point data_start_timer;

	volatile int m_currentQuaternion;
	volatile bool m_update_in_process, m_newQuaternions = false;
	DirectX::XMVECTOR m_renderQuaternion; //the current quaternion to be applied on screen
	std::vector<glm::quat> m_quaternions;
	std::vector<ClubEulerAngles> m_eulerAngles; //holds euler angles coming from the Personal Caddie
	std::vector<std::pair<float, float> > m_angularVelocities; //each pair holds the pitch and yaw angular velocities as read from the sensor
	glm::quat m_headingOffset = { 1.0f, 0.0f, 0.0f, 0.0f };
	std::vector<float> m_timeStamps; //helps figure out which quaternion to actually render (depends on screen refresh rate)
	float m_sensorODR = 0.0f;
	bool m_converged;
	std::vector<glm::quat> m_convergenceQuaternions;

	//Swing Path Training Mode Specific Variables
	std::vector<DirectX::XMFLOAT2> m_swingPath; //Tracks the club path through the impact zone
	float m_tangential_swing_speed, m_radial_swing_speed;
	int m_modeType; //there are both training and game modes
	int m_currentLevel, m_currentStreak, m_toLevelUp;
	std::pair<float, float> m_threshold; //the upper and lower limit (in degrees) that the swing can be off by and still get a pass
	std::pair<float, float> m_goalLimits; //the upper and lower bound that the goal angle stays between
	float m_goalAngle;

	//Array used to swap real world coordinates to DirectX coordinates
	int computer_axis_from_sensor_axis[3] = {1, 2, 0};
};