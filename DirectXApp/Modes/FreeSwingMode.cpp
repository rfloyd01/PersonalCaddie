#include "pch.h"
#include "FreeSwingMode.h"
#include "Graphics/Objects/3D/Elements/Model.h"
#include "Math/quaternion_functions.h"

FreeSwingMode::FreeSwingMode()
{
	//set a very light gray background color for the mode
	m_backgroundColor = UIColor::FreeSwingMode;
}

uint32_t FreeSwingMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Create UI Elements on the page
	initializeTextOverlay();

	//load the model of the golf club
	loadModel();

	//Load the quaternion vector with default quaternions and the 
	//time stamp vector with default times. We use 39 as this is
	//the maximum number of samples we can have, this will get 
	//pruned down when the first bit of data actuall comes in.
	for (int i = 0; i < 39; i++)
	{
		m_quaternions.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		m_eulerAngles.push_back({ 0.0f, 0.0f, 0.0f });
		m_angularVelocities.push_back({ 0.0f, 0.0f });
		m_timeStamps.push_back(0.0f);
	}
	m_renderQuaternion = { m_quaternions[0].x, m_quaternions[0].y, m_quaternions[0].z, m_quaternions[0].w };

	m_converged = false; //We need to let the filter re-converge every time this mode is opened
	m_convergenceQuaternions = {};

	//We spend the entirety of our time in this mode with the Personal Caddie in Sensor Active
	//Mode. To get there we need to first put the Sensor into Idle mode
	auto mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);

	//Set the swing phase to be the start of the swing. This will allow
	//proper initialization of swing start time and club Euler Angles. Since
	//Euler Angles are going to be used, we also alert the Personal Caddie
	//to start calculating them for us.
	m_swing_phase = SwingPhase::START;
	DataType dt = DataType::EULER_ANGLES;
	m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&dt);

	//The NeedMaterial modeState lets the mode screen know that it needs to pass
	//a list of materials to this mode that it can use to initialize 3d objects
	return (ModeState::CanTransfer | ModeState::NeedMaterial | ModeState::Active);
}

void FreeSwingMode::loadModel()
{
	//Create a golf club model
	m_volumeElements.push_back(std::make_shared<Model>());
	((Model*)m_volumeElements[0].get())->loadModel("Assets/Models/golf_club.gltf");

	m_materialTypes.push_back(MaterialType::DEFAULT); //This actually doesn't matter for loading models, but is need to avoid a nullptr exception

	//Set the mesh and materials for the model
	m_mode_screen_handler(ModeAction::RendererGetMaterial, nullptr);

	//ALlow 3d rendering
	m_needsCamera = true;
}

void FreeSwingMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();

	for (int i = 0; i < m_volumeElements.size(); i++) m_volumeElements[i] = nullptr;
	m_volumeElements.clear();

	//Put the Personal Caddie back into Connected Mode when leaving this page. This can be 
	//done without going into Sensor Idle Mode first.
	auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);
}

void FreeSwingMode::initializeTextOverlay()
{
	//Title information
	std::wstring title_message = L"Madgwick Filter Testing";
	TextOverlay title(m_uiManager.getScreenSize(), { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiManager.addElement<TextOverlay>(title, L"Title Text");

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to settings menu";
	TextOverlay footnote(m_uiManager.getScreenSize(), { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiManager.addElement<TextOverlay>(footnote, L"Footnote Text");

	//View data message
	std::wstring view_data_message = L"Press Space to Center the Club";
	TextOverlay view_data(m_uiManager.getScreenSize(), { UIConstants::FootNoteTextLocationX - 0.33f, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		view_data_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)view_data_message.length() }, UITextJustification::LowerCenter);
	m_uiManager.addElement<TextOverlay>(view_data, L"View Data Text");
}

void FreeSwingMode::pc_ModeChange(PersonalCaddiePowerMode newMode)
{
	//As soon as we enter Sensor Idle Mode we jump straight into Sensor Active mode. Before that,
	//however, we also get the current Heading for the sensor which is located in the Personal Caddie
	//class and temporarily update the beta value of the Madgewick filter. The heading allows the sensor
	//to line up with the orientation of the computer screen while the beta value allows the sensor to 
	//quickly converge to the correct location.
	if (newMode == PersonalCaddiePowerMode::SENSOR_IDLE_MODE)
	{
		//Get the current Heading Offset
		m_mode_screen_handler(ModeAction::IMUHeading, nullptr);

		//Temporarily increase Madgwick beta gain to 2.5
		float initial_beta_value = 2.5f;
		m_mode_screen_handler(ModeAction::MadgwickUpdateFilter, (void*)&initial_beta_value);

		//Put the Sensor into Active mode to start taking readings
		auto mode = PersonalCaddiePowerMode::SENSOR_ACTIVE_MODE;
		m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);
	}
}

void FreeSwingMode::addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t)
{
	//It's possible that the quaternions vector will have some empty/junk values at the end. This is because the number of 
	//samples coming from the sensor is dependent on the current sensor ODR (which can change). Because of this we 
	//only add the first quaternion_number quaternions to the vector on this page

	//make sure that the length of the m_quaternion and m_timestamp vectors are the same as the quaternion_number parameter.
	if (m_quaternions.size() != quaternion_number)
	{
		m_quaternions.erase(m_quaternions.begin() + quaternion_number, m_quaternions.end());
		m_timeStamps.erase(m_timeStamps.begin() + quaternion_number, m_timeStamps.end());
	}

	m_currentQuaternion = -1; //reset the current quaternion to be rendered
	m_update_in_process = true; //will get set to false after the addData method below completes

	for (int i = 0; i < quaternion_number; i++)
	{
		m_quaternions[i] = quaternions[i];
		m_timeStamps[i] = time_stamp + i * delta_t;
	}

	data_start_timer = std::chrono::steady_clock::now(); //set relative time

	if (!m_converged)
	{
		//if the filter hasn't yet converged add the first quaternion from this set to the convergence array
		//and call the conergenceCheck() method
		m_convergenceQuaternions.push_back(m_quaternions[0]);
		convergenceCheck();
	}
}

void FreeSwingMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples)
{
	//For now the only thing we care about data wise are the Euler Angles as these
	//tell us what phase of the swing we're currently in.

	//make sure that the length of the m_eulerAngles vector is the same as the total samples coming in.
	if (m_eulerAngles.size() != totalSamples)
	{
		m_eulerAngles.erase(m_eulerAngles.begin() + totalSamples, m_eulerAngles.end());
		m_angularVelocities.erase(m_angularVelocities.begin() + totalSamples, m_angularVelocities.end());
	}

	m_sensorODR = sensorODR; //this value should always stay the same, this is just the best place to originally set it

	int euler_angle_index = static_cast<int>(DataType::EULER_ANGLES);
	int angular_velocity_index = static_cast<int>(DataType::ROTATION);
	for (int i = 0; i < totalSamples; i++)
	{
		m_eulerAngles[i] = { sensorData[euler_angle_index][X][i], sensorData[euler_angle_index][Y][i] , sensorData[euler_angle_index][Z][i] };
		m_angularVelocities[i] = {sensorData[angular_velocity_index][Y][i], sensorData[angular_velocity_index][Z][i] };
	}

	m_update_in_process = false; //was initially set to true in the above addQuaternions() method
}

void FreeSwingMode::update()
{
	//Animate the current rotation quaternion obtained from the Personal Caddie. We need to look at the 
	//time stamp to figure out which quaternion is correct. We do this since the ODR of the sensors won't always
	//match up with the frame rate of the current screen.
	
	while (m_update_in_process) {}; //data is currently being updated asynchronously, wait for it to finish

	float time_elapsed_since_data_start = (float)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - data_start_timer).count() / 1000000000.0f;
	float quat[3];
	bool updated = false;

	for (int i = (m_currentQuaternion + 1); i < m_quaternions.size(); i++)
	{
		if (time_elapsed_since_data_start >= (m_timeStamps[i] - m_timeStamps[0]))
		{
			//since the relative timer is greater than the time distance between the current quaternion
			//and the data set start time, this quaternion can potentially be rendered.
			m_currentQuaternion = i;
			updated = true; //set flag to update render quaternion);
		}
		else break; //we haven't reached the current quaternion in time yet so break out of loop

	}

	if (updated)
	{
		//Rotate the current quaternion from the Madgwick filter by the heading offset to line up with the computer monitor.
		glm::quat adjusted_q;
		adjusted_q = QuaternionMultiply(m_headingOffset, m_quaternions[m_currentQuaternion]);

		float Q_sensor[3] = { adjusted_q.x, adjusted_q.y, adjusted_q.z };
		float Q_computer[3] = { Q_sensor[computer_axis_from_sensor_axis[0]], Q_sensor[computer_axis_from_sensor_axis[1]], Q_sensor[computer_axis_from_sensor_axis[2]] };

		m_renderQuaternion = { Q_computer[0], Q_computer[1], Q_computer[2], adjusted_q.w };
		m_current_club_angles = m_eulerAngles[m_currentQuaternion]; //Update the current euler angles for the club as well
	}

	//Rotate each face according to the given quaternion
	for (int i = 0; i < m_volumeElements.size(); i++) ((Model*)m_volumeElements[i].get())->translateAndRotateFace({ 0.0f, 0.0f, 1.0f }, m_renderQuaternion);

	//TEST: Once all visual updates are complete check the current swing phase
	swingUpdate();
}

void FreeSwingMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
{
	//There are few different keys we can process here. Like in the other modes, pressing the escape
	//key will go back to the previous menu. Pressing the space key will update the heading offset for
	//the Personal Caddie, pressing enter will toggle a live data stream, pressing the number keys will
	//alter the data seen on the stream, and pressing the up arrow will switch between different implementations
	//of the Madgwick filter.
	switch (pressedKey)
	{
	case winrt::Windows::System::VirtualKey::Escape:
	{
		ModeType newMode = ModeType::MAIN_MENU;
		m_mode_screen_handler(ModeAction::ChangeMode, (void*)&newMode);
		break;
	}
	case winrt::Windows::System::VirtualKey::Space:
	{
		setCurrentHeadingOffset();
		break;
	}
	}
}

void FreeSwingMode::getIMUHeadingOffset(glm::quat heading)
{
	//This method gets the heading offset saved in the IMU class and updates
	//the local heading offset variable with it. This variable is used to make
	//sure the rendered image aligns with the computer monitor
	m_headingOffset = heading;
}

void FreeSwingMode::setCurrentHeadingOffset()
{
	//The Madgwick filter uses due North as a reference for the magnetic field, so if the computer monitor
	//isn't aligned with this direction then the sensor will appear to have an improper heading while being
	//rendered on screen. This function can be called to get the current heading offset from due North
	//which can in turn be used to align the calculated reference direction of North from the Madwick filter
	//to align with the computer monitor. All of the following calculations use the sensor coordinate frame
	//(where +Z axis is up instead of +Y).

	//We rotate a vector representing true North by the current rotation quaternion. We then project the resulting
	//vector back into the XY plane, and rotate it back to due North. The rotation required to get back to due
	//North is the value that we return.

	glm::quat trueNorth = { 0, 1, 0, 0 }; //Madgwick filter has North aligned with the +X axis
	glm::quat currentHeading = QuaternionMultiply(m_quaternions[0], trueNorth); //use the first quaternion in the array
	currentHeading = QuaternionMultiply(currentHeading, { m_quaternions[0].w, -m_quaternions[0].x, -m_quaternions[0].y, -m_quaternions[0].z}); //use the first quaternion in the array

	//Project the current heading into the XY plane (using sensor coordinates, meaning Z is set to 0)
	currentHeading.z = 0;

	//Calculate the angle from the current heading to true North by calculating the cross product
	float angle = asin(CrossProduct({ currentHeading.x, currentHeading.y, currentHeading.z }, {trueNorth.x, trueNorth.y, trueNorth.z})[2] / sqrt(currentHeading.x * currentHeading.x + currentHeading.y * currentHeading.y));

	//return the proper rotation quaternion about the y-axis as opposed to the z-axis
	//as it gets applied after the Madgwick filter (so +y is up instead of +z)
	m_headingOffset = { cos(angle / 2.0f), 0.0f, 0.0f, sin(angle / 2.0f)};
	m_mode_screen_handler(ModeAction::IMUHeading, (void*)&m_headingOffset);
}

void FreeSwingMode::convergenceCheck()
{
	//We check to see if the filter has converged so that we can update the filter's beta value
	//to something more useful. To check for convergence, we average the last 10 quaternions together
	//and check the error between this and the current quaternion. If the error in the w, x, y and z
	//fields are all below a certain threshold then the convergence check passes and we reset the
	//beta value of the filter.
	if (m_convergenceQuaternions.size() >= 10)
	{
		glm::quat averageQuaternion = { 0, 0, 0, 0 };
		float error_threshold = 0.05f;

		for (int i = m_convergenceQuaternions.size() - 10; i < m_convergenceQuaternions.size(); i++) averageQuaternion += m_convergenceQuaternions[i];
		averageQuaternion /= 10;

		float w_error = (averageQuaternion.w - m_convergenceQuaternions.back().w) / (averageQuaternion.w + m_convergenceQuaternions.back().w);
		float x_error = (averageQuaternion.x - m_convergenceQuaternions.back().x) / (averageQuaternion.x + m_convergenceQuaternions.back().x);
		float y_error = (averageQuaternion.y - m_convergenceQuaternions.back().y) / (averageQuaternion.y + m_convergenceQuaternions.back().y);
		float z_error = (averageQuaternion.z - m_convergenceQuaternions.back().z) / (averageQuaternion.z + m_convergenceQuaternions.back().z);

		if (w_error >= 1.0f) w_error = 1.0f / w_error;

		if (w_error > error_threshold || w_error < -error_threshold) return;
		if (x_error > error_threshold || x_error < -error_threshold) return;
		if (y_error > error_threshold || y_error < -error_threshold) return;
		if (z_error > error_threshold || z_error < -error_threshold) return;

		//If all error check pass we reset the beta value of the filter to once again bias the gyro readings
		float initial_beta_value = 0.041f;
		m_mode_screen_handler(ModeAction::MadgwickUpdateFilter, (void*)&initial_beta_value);;

		//And do a little clean up
		m_convergenceQuaternions.clear();
		m_converged = true; //prevents this convergenceCheck() from being called again
	}
}

void FreeSwingMode::swingUpdate()
{
	switch (m_swing_phase)
	{
	case SwingPhase::START:
	{
		//In this phase all we do is set the initial euler angles for the club and
		//the start time-stamp for the swing. This only happens though if the 
		//Madgwick filter has properly converged on the club's real world posiiton.
		if (m_converged)
		{
			m_initial_club_angles = m_current_club_angles;
			m_swing_start_time = std::chrono::steady_clock::now();
			m_swing_phase = SwingPhase::PRE_ADDRESS;
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

			Ellipse address_ellipse(m_uiManager.getScreenSize(), { 0.167f, 0.25f }, { MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * 0.033f, 0.033f }, false, UIColor::Red);
			m_uiManager.addElement<Ellipse>(address_ellipse, L"Ellipse 1");

			//Create a vector pointing along the shaft of the club at address. This will 
			//help us detect when we're close to impact with the ball later on.
			m_ball_location = { 1.0f, 0.0f, 0.0f };
			auto currentQuat = QuaternionMultiply(m_headingOffset, m_quaternions[m_currentQuaternion]);
			QuatRotate(currentQuat, m_ball_location);
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

			Ellipse backswing_ellipse(m_uiManager.getScreenSize(), { 0.333f, 0.25f }, { MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * 0.033f, 0.033f }, false, UIColor::Orange);
			m_uiManager.addElement<Ellipse>(backswing_ellipse, L"Ellipse 2");
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
			m_current_pitch_average += m_angularVelocities[m_currentQuaternion].first;
			m_current_yaw_average += m_angularVelocities[m_currentQuaternion].second;
			m_backswing_point++;
		}
		else
		{
			if (detectTransition(m_previous_pitch_average, m_current_pitch_average, m_previous_yaw_average, m_current_yaw_average, TRANSITION_MOVING_AVERAGE_POINTS / m_sensorODR))
			{
				m_swing_phase = SwingPhase::TRANSITION;
				
				Ellipse transition_ellipse(m_uiManager.getScreenSize(), { 0.5f, 0.25f }, { MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * 0.033f, 0.033f }, false, UIColor::Yellow);
				m_uiManager.addElement<Ellipse>(transition_ellipse, L"Ellipse 3");
			}
			else
			{
				m_backswing_point = 0;
			}

			m_previous_pitch_average = m_current_pitch_average;
			m_previous_yaw_average = m_current_yaw_average;
			m_current_pitch_average = 0.0f;
			m_current_yaw_average = 0.0f;
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
		if (detectDownswing(m_previous_pitch_average, m_current_pitch_average, m_previous_yaw_average, m_current_yaw_average, TRANSITION_MOVING_AVERAGE_POINTS / m_sensorODR))
		{
			m_swing_phase = SwingPhase::DOWNSWING;
			
			Ellipse downswing_ellipse(m_uiManager.getScreenSize(), { 0.667f, 0.25f }, { MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * 0.033f, 0.033f }, false, UIColor::Green);
			m_uiManager.addElement<Ellipse>(downswing_ellipse, L"Ellipse 4");
		}
		else
		{
			m_previous_pitch_average = m_current_pitch_average;
			m_previous_yaw_average = m_current_yaw_average;
			m_current_pitch_average = 0.0f;
			m_current_yaw_average = 0.0f;
			m_backswing_point = 0;
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
		for (int i = 0; i < m_quaternions.size(); i++)
		{
			//Remember to rotate the current quaternion by the heading offset before checking
			//for impact
			if (detectImpact(m_ball_location, QuaternionMultiply(m_headingOffset, m_quaternions[0])))
			{
				m_swing_phase = SwingPhase::IMPACT;

				Ellipse impact_ellipse(m_uiManager.getScreenSize(), { 0.833f, 0.25f }, { MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * 0.033f, 0.033f }, false, UIColor::Blue);
				m_uiManager.addElement<Ellipse>(impact_ellipse, L"Ellipse 5");
			}
		}

		break;
	}
	}
}