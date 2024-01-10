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
		m_timeStamps.push_back(0.0f);
	}
	m_renderQuaternion = { m_quaternions[0].x, m_quaternions[0].y, m_quaternions[0].z, m_quaternions[0].w };

	m_converged = false; //We need to let the filter re-converge every time this mode is opened
	m_convergenceQuaternions = {};

	//We spend the entirety of our time in this mode with the Personal Caddie in Sensor Active
	//Mode. To get there we need to first put the Sensor into Idle mode
	auto mode = PersonalCaddiePowerMode::SENSOR_IDLE_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);

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
	m_update_in_process = true;

	for (int i = 0; i < quaternion_number; i++)
	{
		m_quaternions[i] = quaternions[i];
		m_timeStamps[i] = time_stamp + i * delta_t;
	}

	data_start_timer = std::chrono::steady_clock::now(); //set relative time

	m_update_in_process = false;

	if (!m_converged)
	{
		//if the filter hasn't yet converged add the first quaternion from this set to the convergence array
		//and call the conergenceCheck() method
		m_convergenceQuaternions.push_back(m_quaternions[0]);
		convergenceCheck();
	}
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
	}

	//Rotate each face according to the given quaternion
	for (int i = 0; i < m_volumeElements.size(); i++) ((Model*)m_volumeElements[i].get())->translateAndRotateFace({ 0.0f, 0.0f, 1.0f }, m_renderQuaternion);
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