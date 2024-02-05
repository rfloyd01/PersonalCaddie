#include "pch.h"

#include "SwingPhaseDetection.h"
#include "Math/quaternion_functions.h"
#include <chrono>

bool detectAddress(ClubEulerAngles& initial_angles, ClubEulerAngles& current_angles, std::chrono::time_point<std::chrono::steady_clock>& start_time)
{
	//For the club to be considered at address the roll and pitch angles of the club need
	//to stay within a given window. Furthermore, once inside of this window the club must
	//stay relative still over a set period of time.
	auto current_time = std::chrono::steady_clock::now();

	//First, make sure that the club is in the address window
	if (current_angles.pitch > ADDRESS_MAX_PITCH_THRESHOLD || current_angles.pitch < ADDRESS_MIN_PITCH_THRESHOLD ||
		current_angles.roll > ADDRESS_MAX_ROLL_THRESHOLD || current_angles.roll < ADDRESS_MIN_ROLL_THRESHOLD)
	{
		//Since the club isn't in the necessary window, reset the given address start time,
		//update the club's initial angles and return false
		start_time = current_time;
		initial_angles = current_angles;
		return false;
	}

	//If the club is within the address window, see if it's moved beyond the allowed threshold
	//(i.e. during a club waggle or something similar). If so, reset the swing start_time and 
	//return false.
	float roll_delta = current_angles.roll - initial_angles.roll;
	float pitch_delta = current_angles.pitch - initial_angles.pitch;
	float yaw_delta = current_angles.yaw - initial_angles.yaw;

	if ((roll_delta > ADDRESS_ANGLE_THRESHOLD || roll_delta < -ADDRESS_ANGLE_THRESHOLD) || (pitch_delta > ADDRESS_ANGLE_THRESHOLD || pitch_delta < -ADDRESS_ANGLE_THRESHOLD) ||
		(yaw_delta > ADDRESS_ANGLE_THRESHOLD || yaw_delta < -ADDRESS_ANGLE_THRESHOLD))
	{
		//Although the club is within the proper window, it's moved too much to be considered
		//at address. Update the start_time, the club's initial angles and return false
		start_time = current_time;
		initial_angles = current_angles;
		return false;
	}

	//If the club is within the address window and is relatively stationary to when the swing
	//first started, see if it's been stationary for the necessary time period to be considered
	//at address.
	float delta_t = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
	if (delta_t >= ADDRESS_TIME_THRESHOLD_MS)
	{
		return true;
	}
	else return false; //no need to update the initial start time or club angles as they're currently good
}

bool detectBackswing(ClubEulerAngles& initial_angles, ClubEulerAngles& current_angles)
{
	//For backswing detection all we do is wait for the club to tilt backwards from the ball
	//by a set amount. The angle for this is the same as the angles the club had to stay inside
	//of before it was considered at address. As the name implies, the "backswing" must be in
	//the direction away from the target, so they only Euler Angle we check here it the pitch
	//angle.

	//TODO: This currently only makes sense for a righty golfer. At some point in the future
	//I should add a lefty option (but this is obviously quite a ways off)
	if (current_angles.yaw >= (initial_angles.yaw + ADDRESS_ANGLE_THRESHOLD))
	{
		return true;
	}
	return false;
}

bool detectTransition(float previous_pitch, float current_pitch, float previous_yaw, float current_yaw, float delta_t)
{
	//The transition phase of the golf swing is a little more abstract than other phases. It's called
	//the transition because it's where the club "transitions" from moving backwards to moving forwards.
	//Transitions can be short and abrupt, long and smooth, or anywhere in between. Regardless of 
	//what the move looks like in real world though, it can be characterized with data by looking 
	//at the angular velocity rates along the pitch and yaw axes. The angular rate along both of these
	//axes will spike upwards at the beginning of the backswing and then return back to 0 once the 
	//top of the backswing is reached. Because of this, we flag the transition phase as starting when
	//both of these angular rates start to decrease from their maximum values.

	//First calculate the slopes for the rotation rates to see how quickly they're changing
	float pitch_slope = (current_pitch - previous_pitch) / delta_t;
	float yaw_slope = (current_yaw - previous_yaw) / delta_t;

	//It's not enough to just look at the slope for the data (as depending on how the axes of the
	//sensors are set up these can be positive or negative). Any data point that's positive will
	//need a negative slope to get back to 0, likewise, any data point that's currently negative
	//will need to have a positive slope to get back to 0. If both sets of data are actively
	//returning to 0 then we return true to indicate that the transition phase has begun.
	if ((current_pitch > 0 && pitch_slope > 0) || (current_pitch < 0 && pitch_slope < 0) ||
		(current_yaw > 0 && yaw_slope > 0) || (current_yaw < 0 && yaw_slope < 0)) return false;

	return true;
}


bool detectDownswing(float previous_pitch, float current_pitch, float previous_yaw, float current_yaw, float delta_t)
{
	//The start of the downswing is calculated much in the same way that the start of
	//transition phase is. The only difference here is that we wait for the angular 
	//velocity data to invert along both axis. So if the rate of change in angular 
	//velocity is negative, this means we wait for the angular velocity to move from a 
	//positive number to a negative number. If the rate of change is positive, we do the 
	//opposite and wait for the angular velocity to flip from negativeto positive

	//Calculate the rate of change (slopes) for the angular velocities
	float pitch_slope = (current_pitch - previous_pitch) / delta_t;
	float yaw_slope = (current_yaw - previous_yaw) / delta_t;

	if ((pitch_slope > 0 && current_pitch < 0) || (pitch_slope < 0 && current_pitch > 0) ||
		(yaw_slope > 0 && current_yaw < 0) || (yaw_slope > 0 && current_yaw < 0)) return false;

	return true;
}

bool detectImpact(std::vector<float> const& ball_location, glm::quat const& quaternion)
{
	//When we first address the ball during the swing, the location of the ball is saved as a vector
	//that points from the origin to the ball and is normalized to have a length of 1. The way that we detect
	//impact is by rotating the same unit vector by the "quaternion" parameter and seeing if the distance
	//between the end of this new unit vector and the ball's unit vector is within a certain threshold distance.
	//If so, we officially move on to the impact phase.
	std::vector<float> club_vector = { 1.0f, 0.0f, 0.0f }; //starts off as a unit vector pointing down the x-axisd
	QuatRotate(quaternion, club_vector);

	float x_distance = club_vector[0] - ball_location[0];
	float y_distance = club_vector[1] - ball_location[1];
	float z_distance = club_vector[2] - ball_location[2];
	float total_distance = sqrt(x_distance * x_distance + y_distance * y_distance + z_distance * z_distance);

	std::wstring debug = L"Distance from ball = " + std::to_wstring(total_distance) + L"\n";
	OutputDebugString(&debug[0]);

	if (total_distance <= IMPACT_DISTANCE_THRESHOLD) return true;
	return false;
}