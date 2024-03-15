#pragma once

/*
This class should be implemented by any modes where some kind of analysis
of a golf swing needs to happen. It uses the SwingPhaseDetection class to 
figure out the current phase of the golf swing we're currently in, and 
provides virtual methods that will automatically be accessed during each of
the phases. It's up to classes implementing this one to define what actions,
if any, need to occur at each phase of the golf swing.
*/

#include "SwingPhaseDetection.h"

class GolfSwing
{
public:
	GolfSwing() {} //empty default constructor

	void setGolfSwingReferenceVariables(volatile int* currentQuaternion, volatile bool* newQuaternions, bool* converged, glm::quat* headingOffset,
		std::vector<glm::quat>* quaternions, std::vector<std::pair<float, float> >* angularVelocities, float* sensorODR)
	{
		//For the golf swing to progress properly there are a few reference variables
		//that the swing will need from the mode that's inheriting it
		p_currentQuaternion = currentQuaternion;
		p_converged = converged;
		p_headingOffset = headingOffset;
		p_newQuaternions = newQuaternions;
		p_quaternions = quaternions;
		p_angularVelocities = angularVelocities;
		p_sensorODR = sensorODR;
	}

	void swingUpdate();
	void setInitialClubAngles(ClubEulerAngles& club_angles);
	void updateClubAngles(ClubEulerAngles& club_angles);
	void updateEulerPitchAverage(float pitch_average);
	void updateEulerYawAverage(float yaw_average);
	void setTargetLine(glm::quat const& target_heading);

protected:

	//Virtual methods with no behaviour. This makes it so inheriting classes
	//only need to provide implementations for the methods they need.
	virtual void swingStartAction() {}; //acts as the pre-pre_address action
	virtual void pre_AddressAction() {};
	virtual void addressAction() {};
	virtual void backswingAction() {};
	virtual void transitionAction() {};
	virtual void downswingAction() {};
	virtual void impactAction() {};
	virtual void followThroughAction() {};
	virtual void swingEndAction() {};

	//The following virtual methods only get called a single time as we move 
	//from one phase of the swing to the next. Like the above methods they 
	//have a default blank implementation so that the implementing class doesn't
	//need to provide an action for any method that it doesn't need.
	virtual void preAddressAction() {};
	virtual void preBackswingAction() {};
	virtual void preTransitionAction() {};
	virtual void preDownswingAction() {};
	virtual void preImpactAction() {};
	virtual void preFollowThroughAction() {};
	virtual void preSwingEndAction() {};

	//Reference Variables -- These are variables which are owned by the class which extends
	//the golf swing class, however, can't be directly accessed here
	volatile int* p_currentQuaternion;
	glm::quat* p_headingOffset;
	volatile bool* p_newQuaternions;
	std::vector<glm::quat>* p_quaternions;
	std::vector<std::pair<float, float> >* p_angularVelocities;
	bool* p_converged;
	float* p_sensorODR;

	//Data Variables
	ClubEulerAngles m_current_club_angles, m_initial_club_angles;
	float m_previous_pitch_average, m_current_pitch_average;
	float m_previous_yaw_average, m_current_yaw_average;
	glm::quat m_targetLineHeading = { 1.0f, 0.0f, 0.0f, 0.0f }; //heading difference between target line at address and heading offset

	//Phase detection variables
	SwingPhase m_swing_phase;
	std::chrono::time_point<std::chrono::steady_clock> m_swing_start_time;
	std::vector<float> m_ball_location;
	int m_backswing_point; //keeps track of which point is being looked at for the data average
};