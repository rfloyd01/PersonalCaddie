#include "pch.h"

#include "GolfSwing.h"
#include "Math/quaternion_functions.h"

void GolfSwing::swingUpdate()
{
	switch (m_swing_phase)
	{
	case SwingPhase::START:
	{
		//In this phase all we do is set the initial euler angles for the club and
		//the start time-stamp for the swing. This only happens though if the 
		//Madgwick filter has properly converged on the club's real world posiiton.
		if (p_converged)
		{
			m_initial_club_angles = m_current_club_angles;
			m_swing_start_time = std::chrono::steady_clock::now();
			m_swing_phase = SwingPhase::PRE_ADDRESS;
			swingStartAction(); //One time action before pre-address starts
		}
		break;
	}
	case SwingPhase::PRE_ADDRESS:
	{
		//In this phase of the swing we're waiting for the golfer to put the club head
		//behind the ball and remain relatively still. Most golfer's have the tendency 
		//to waggle the club a few times before they fully address the ball, which will
		//happen in this phase.
		if (detectAddress(m_initial_club_angles, m_current_club_angles, m_swing_start_time))
		{
			//Once the golfer has stopped moving within a certain threshold, we move on
			//to the address portion of the swing
			m_swing_phase = SwingPhase::ADDRESS;
			m_initial_club_angles = m_current_club_angles; //the initial angles now mark the address angles

			//Create a vector pointing along the shaft of the club at address. This will 
			//help us detect when we're close to impact with the ball later on.
			m_ball_location = { 1.0f, 0.0f, 0.0f };
			auto currentQuat = QuaternionMultiply(*p_headingOffset, p_quaternions->at(*p_currentQuaternion));
			QuatRotate(currentQuat, m_ball_location);

			preAddressAction(); //One time action before address starts
		}
		else
		{
			pre_AddressAction(); //Action that gets called in every iteration during pre-address
		}
		break;
	}
	case SwingPhase::ADDRESS:
	{
		//In the address phase of the swing all we're really doing is waiting
		//for the golfer to initiate the swing which is a simple check.
		if (detectBackswing(m_initial_club_angles, m_current_club_angles))
		{
			m_swing_phase = SwingPhase::BACKSWING;
			m_initial_club_angles = m_current_club_angles; //the initial angles now mark the address angles

			//Set some variables needed during backswing detection
			m_previous_pitch_average = 0.0f;
			m_previous_yaw_average = 0.0f;
			m_current_pitch_average = 0.0f;
			m_current_yaw_average = 0.0f;
			m_backswing_point = 0; //set the backswing average index to 0

			preBackswingAction(); //One time action before backswing starts
		}
		else
		{
			addressAction(); //Action that gets called in every iteration during address
		}
		break;
	}
	case SwingPhase::BACKSWING:
	{
		//During the backswing we look at the angular velocity along the pitch and yaw
		//axes. They should both increase (in either the positive or negative direction)
		//before reaching a peak and then going back towards 0. The transition phase is
		//entered when both of these angular velocities get close enough to 0. Since
		//the data is noisy a moving average is used.
		if (m_backswing_point < TRANSITION_MOVING_AVERAGE_POINTS)
		{
			m_current_pitch_average += p_angularVelocities->at(*p_currentQuaternion).first;
			m_current_yaw_average += p_angularVelocities->at(*p_currentQuaternion).second;
			m_backswing_point++;
		}
		else
		{
			if (detectTransition(m_previous_pitch_average, m_current_pitch_average, m_previous_yaw_average, m_current_yaw_average, TRANSITION_MOVING_AVERAGE_POINTS / *p_sensorODR))
			{
				m_swing_phase = SwingPhase::TRANSITION;

				preTransitionAction(); //One time action before transition starts
			}
			else
			{
				m_backswing_point = 0;
			}

			m_previous_pitch_average = m_current_pitch_average;
			m_previous_yaw_average = m_current_yaw_average;
			m_current_pitch_average = 0.0f;
			m_current_yaw_average = 0.0f;

			backswingAction(); //Action that gets called in every iteration during backswing
		}
		break;
	}
	case SwingPhase::TRANSITION:
	{
		//The transition is usually pretty short in comparison to the backswing and 
		//downswing. The end of the phase is calculated in a pretty similar way that
		//the beginning of the phase was. We wait until the angular velocity along the
		//pitch and yaw axes have both inverted from the beginning of the phase which
		//signals the start of the downswing.
		if (detectDownswing(m_previous_pitch_average, m_current_pitch_average, m_previous_yaw_average, m_current_yaw_average, TRANSITION_MOVING_AVERAGE_POINTS / *p_sensorODR))
		{
			m_swing_phase = SwingPhase::DOWNSWING;

			preDownswingAction(); //One time action before downswing starts
		}
		else
		{
			m_previous_pitch_average = m_current_pitch_average;
			m_previous_yaw_average = m_current_yaw_average;
			m_current_pitch_average = 0.0f;
			m_current_yaw_average = 0.0f;
			m_backswing_point = 0;

			transitionAction(); //Action that gets called in every iteration during transition
		}

		break;
	}
	case SwingPhase::DOWNSWING:
	{
		//The downswing is everything that occurs between the transition and impact
		//phases of the swing. Since we want to see a little bit before impact to get
		//a sense of what direction the club is travelling, the downswing phase ends
		//a few milliseconds before the club actually hits the ball. Furthermore,
		//since the club is moving so fast in the downswing we want to use every single
		//quaternion that's available to us, not just the one being rendered on the screen.
		//Because of this, we iterate directly through the quaternion vector when sensing
		//proximity to impact.
		if (!(*p_newQuaternions)) return; //no need to recheck the same quaternions

		for (int i = 0; i < p_quaternions->size(); i++)
		{
			//Remember to rotate the current quaternion by the heading offset before checking
			//for impact
			if (detectImpact(m_ball_location, QuaternionMultiply(*p_headingOffset, p_quaternions->at(i))))
			{
				m_swing_phase = SwingPhase::IMPACT;

				preImpactAction(); //One time action before impact starts
				break;
			}
		}

		downswingAction(); //Action that gets called in every iteration during downswing

		*p_newQuaternions = false; //setting this bool prevents from checking the same points over and over

		break;
	}
	case SwingPhase::IMPACT:
	{
		//This phase is very similar to the downswing phase in that it happens very quickly
		//so we look at every single quaternion for information. The difference here is we
		//save position data from each of these quaternions to get important info about
		//the contact with the ball (things like swing speed and direction of the club head).
		//Once the club travels a certain distance past the ball we move on to the final 
		//phase of the swing
		if (!(*p_newQuaternions)) return; //no need to recheck the same quaternions

		for (int i = 0; i < p_quaternions->size(); i++)
		{
			std::vector<float> club_orientation = { 1.0f, 0.0f, 0.0f }; //will get rotated according to the current quaternion

			if (detectFollowThrough(m_ball_location, club_orientation, QuaternionMultiply(*p_headingOffset, p_quaternions->at(i))))
			{
				m_swing_phase = SwingPhase::FOLLOW_THROUGH;

				//Set some variables needed for follow through end detection
				m_previous_pitch_average = 0.0f;
				m_previous_yaw_average = 0.0f;
				m_current_pitch_average = 0.0f;
				m_current_yaw_average = 0.0f;
				m_backswing_point = 0;

				preFollowThroughAction(); //One time action before impact starts
			}
		}

		impactAction(); //Action that gets called in every iteration during impact

		*p_newQuaternions = false; //setting this bool prevents from checking/adding the same points over and over

		break;
	}
	case SwingPhase::FOLLOW_THROUGH:
	{
		//After impact, the club will travel upwards some distance before coming to a stop, this
		//is known as the follow through. We use this phase of the swing to create graphs from the 
		//information gathered during impact.

		if (detectSwingEnd(m_previous_pitch_average, m_current_pitch_average, m_previous_yaw_average, m_current_yaw_average, TRANSITION_MOVING_AVERAGE_POINTS / *p_sensorODR))
		{
			m_swing_phase = SwingPhase::END;

			preSwingEndAction(); //One time action before the swing ends
		}
		else
		{
			m_previous_pitch_average = m_current_pitch_average;
			m_previous_yaw_average = m_current_yaw_average;
			m_current_pitch_average = 0.0f;
			m_current_yaw_average = 0.0f;
			m_backswing_point = 0;

			followThroughAction(); //Action that gets called in every iteration during follow through
		}

		break;
	}
	case SwingPhase::END:
	{
		//Once the swing is over we simply go back to the start and do everything again.
		m_swing_phase = SwingPhase::START;
		swingEndAction(); //One time action that gets called once the swing is officially over
		break;
	}
	}
}

void GolfSwing::setInitialClubAngles(ClubEulerAngles& club_angles)
{
	m_initial_club_angles = club_angles;
}

void GolfSwing::updateClubAngles(ClubEulerAngles& club_angles)
{
	m_current_club_angles = club_angles;
}

void GolfSwing::updateEulerPitchAverage(float pitch_average)
{
	m_previous_pitch_average = m_current_pitch_average;
	m_current_pitch_average = pitch_average;
}

void GolfSwing::updateEulerYawAverage(float yaw_average)
{
	m_previous_yaw_average = m_current_yaw_average;
	m_current_yaw_average = yaw_average;
}

void GolfSwing::setTargetLine(glm::quat const& target_heading)
{
	//It's possible that when the golfer comes to address that they aren't perfectly
	//in line with the computer screen. This can potentially skew any swing path data
}