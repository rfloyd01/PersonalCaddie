#pragma once

/*
The golf swing can be broken up into a few distinct phases: address, backswing,
transition, downswing, impact and follow through. This class has methods that
detect when each of these phases is entered during a golf swing.
*/

#include "Math/glm.h"

//Definitions
#define DEGREES_TO_RADIANS 0.017453278f //pi / 180

#define ADDRESS_TIME_THRESHOLD_MS 2000.0f
#define ADDRESS_ANGLE_THRESHOLD (5.0f * DEGREES_TO_RADIANS)
#define ADDRESS_MAX_PITCH_THRESHOLD (80.0f * DEGREES_TO_RADIANS)
#define ADDRESS_MIN_PITCH_THRESHOLD (10.0f * DEGREES_TO_RADIANS)
#define ADDRESS_MAX_ROLL_THRESHOLD (120.0f * DEGREES_TO_RADIANS)
#define ADDRESS_MIN_ROLL_THRESHOLD (60.0f * DEGREES_TO_RADIANS)

#define TRANSITION_MOVING_AVERAGE_POINTS 5
#define TRANSITION_INTERCEPT_THRESHOLD 0.5f //Will need to tweak this value accordingly

#define IMPACT_DISTANCE_THRESHOLD 0.175f

//Structs and Enums
struct ClubEulerAngles
{
	float roll;
	float pitch;
	float yaw;
};

enum class SwingPhase
{
	START,
	PRE_ADDRESS,
	ADDRESS,
	BACKSWING,
	TRANSITION,
	DOWNSWING,
	IMPACT,
	FOLLOW_THROUGH,
	END
};

bool detectAddress(ClubEulerAngles& initial_angles, ClubEulerAngles& current_angles, std::chrono::time_point<std::chrono::steady_clock>& start_time);
bool detectBackswing(ClubEulerAngles& initial_angles, ClubEulerAngles& current_angles);
bool detectTransition(float previous_pitch, float current_pitch, float previous_yaw, float current_yaw, float delta_t);
bool detectDownswing(float previous_pitch, float current_pitch, float previous_yaw, float current_yaw, float delta_t);
bool detectImpact(std::vector<float> const& ball_location, glm::quat const& quaternion);
bool detectFollowThrough(std::vector<float> const& ball_location, std::vector<float>& club_orientation, glm::quat const& quaternion);
bool detectSwingEnd(float previous_pitch, float current_pitch, float previous_yaw, float current_yaw, float delta_t);