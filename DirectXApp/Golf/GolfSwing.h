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
	GolfSwing(volatile int* currentQuaternion, volatile bool* newQuaternions, bool* converged, glm::quat* headingOffset,
		std::vector<glm::quat>* quaternions, std::vector<std::pair<float, float> >* angularVelocities, float* sensorODR)
	{
		//For the golf swing to progress properly there are a few reference variables
		//that the swing will need from the mode that's inheriting it
		m_currentQuaternion = currentQuaternion;
		m_converged = converged;
		m_headingOffset = headingOffset;
		m_newQuaternions = newQuaternions;
		m_quaternions = quaternions;
		m_angularVelocities = angularVelocities;
		m_sensorODR = sensorODR;
	}
	void swingUpdate();

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

private:

	//Reference Variables
	volatile int* m_currentQuaternion; //keeps track of the quaternion currently being rendered on the screen
	glm::quat* m_headingOffset;
	volatile bool* m_newQuaternions;
	std::vector<glm::quat>* m_quaternions;
	std::vector<std::pair<float, float> >* m_angularVelocities; //each pair holds the pitch and yaw angular velocities as read from the sensor
	bool* m_converged;
	float* m_sensorODR;

	//Phase detection variables
	SwingPhase m_swing_phase;
	ClubEulerAngles m_current_club_angles, m_initial_club_angles;
	std::chrono::time_point<std::chrono::steady_clock> m_swing_start_time;
	std::vector<float> m_ball_location;
	int m_backswing_point; //keeps track of which point is being looked at for the data average
	float m_previous_pitch_average, m_current_pitch_average;
	float m_previous_yaw_average, m_current_yaw_average;
	std::vector<DirectX::XMFLOAT2> m_swingPath; //Tracks the club path through the impact zone
	float m_tangential_swing_speed, m_radial_swing_speed;
};