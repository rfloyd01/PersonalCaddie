#include "pch.h"
#include "SwingPathTrainingMode.h"
#include "Graphics/Objects/3D/Elements/Model.h"
#include "Math/quaternion_functions.h"

#include <random>
#include <chrono>

SwingPathTrainingMode::SwingPathTrainingMode()
{
	//set a very light gray background color for the mode
	m_backgroundColor = UIColor::SwingPathTraining;

	//set the seed for the random number generator
	srand(std::chrono::steady_clock::now().time_since_epoch().count());
}

uint32_t SwingPathTrainingMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Take the current screen size and pass it to the UIElementManager, this is so that the manager knows
	//how large to make each element.
	m_uiManager.updateScreenSize(windowSize);

	//Create UI Elements on the page
	initializeTextOverlay();
	loadTrainingUI();

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
	DataType dt = DataType::EULER_ANGLES;
	m_mode_screen_handler(ModeAction::PersonalCaddieToggleCalculatedData, (void*)&dt);

	//Add a graph for displaying swing path through impact with the ball,
	//as well as some text overlays to display calculated stats
	/*Graph graph(m_uiManager.getScreenSize(), { 0.25, 0.45 }, { 0.2, 0.2 }, true, UIColor::White, UIColor::Black, false, false);
	m_uiManager.addElement<Graph>(graph, L"Graph");
	m_uiManager.getElement<Graph>(L"Graph")->updateState(UIElementState::Invisible);*/

	//TextOverlay calculated_swing_speed(m_uiManager.getScreenSize(), { 0.25, 0.65 }, { 0.35, 0.075 }, L"Swing Speed = ",
	//	0.5f, { UIColor::White }, { 0, 14 }, UITextJustification::CenterLeft, false);
	//m_uiManager.addElement<TextOverlay>(calculated_swing_speed, L"Swing Speed Text");
	//m_uiManager.getElement<TextOverlay>(L"Swing Speed Text")->updateState(UIElementState::Invisible);

	//Initialize golfswing specific variables
	setGolfSwingReferenceVariables(&m_currentQuaternion, &m_newQuaternions, &m_converged, &m_headingOffset,
		&m_quaternions, &m_angularVelocities, &m_sensorODR);
	m_swing_phase = SwingPhase::START;

	//The NeedMaterial modeState lets the mode screen know that it needs to pass
	//a list of materials to this mode that it can use to initialize 3d objects
	return (ModeState::CanTransfer | ModeState::NeedMaterial | ModeState::Active);
}

void SwingPathTrainingMode::loadModel()
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

void SwingPathTrainingMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	m_uiManager.removeAllElements();

	for (int i = 0; i < m_volumeElements.size(); i++) m_volumeElements[i] = nullptr;
	m_volumeElements.clear();

	//Clear out any existing swing data
	m_swingPath.clear();

	//Put the Personal Caddie back into Connected Mode when leaving this page. This can be 
	//done without going into Sensor Idle Mode first.
	auto mode = PersonalCaddiePowerMode::CONNECTED_MODE;
	m_mode_screen_handler(ModeAction::PersonalCaddieChangeMode, (void*)&mode);
}

void SwingPathTrainingMode::initializeTextOverlay()
{
	//Title information
	std::wstring title_message = L"Swing Path Training";
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

void SwingPathTrainingMode::loadTrainingUI()
{
	//This method is for loading UI Elements that are strictly for
	//SwingPathTrainingMode.
	Graph graph(m_uiManager.getScreenSize(), { 0.15f, 0.4f }, { SCREEN_RATIO * 0.25f, 0.25f }, false, UIColor::White, UIColor::Black, false, false);
	graph.setAxisMaxAndMins({ -1.0f, -1.0f }, { 1.0f, 1.0f });
	m_uiManager.addElement<Graph>(graph, L"Graph");

	std::wstring options = L"Training Mode\nGame Mode";
	DropDownMenu mode_selection(m_uiManager.getScreenSize(), { 0.15f, 0.85f }, { 0.2f, 0.08f }, options, 0.5f, 2);

	m_uiManager.addElement<DropDownMenu>(mode_selection, L"Dropdown Menu 1");

	std::wstring level_message = L"Current Level:";
	std::wstring threshold_message = L"Degree Threshold:";
	std::wstring streak_message = L"Current Streak:";
	std::wstring swing_message = L"Swings to Next Level:";
	std::wstring goal_message = L"Current Goal:";

	TextOverlay level(m_uiManager.getScreenSize(), { 0.85f, 0.35f }, { 0.2f, 0.08f }, level_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)level_message.length() }, UITextJustification::CenterLeft);
	TextOverlay threshold(m_uiManager.getScreenSize(), { 0.85f, 0.425f }, { 0.2f, 0.08f }, threshold_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)threshold_message.length() }, UITextJustification::CenterLeft);
	TextOverlay streak(m_uiManager.getScreenSize(), { 0.85f, 0.50f }, { 0.2f, 0.08f }, streak_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)streak_message.length() }, UITextJustification::CenterLeft);
	TextOverlay swing(m_uiManager.getScreenSize(), { 0.85f, 0.575f }, { 0.2f, 0.08f }, swing_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)swing_message.length() }, UITextJustification::CenterLeft);
	TextOverlay goal(m_uiManager.getScreenSize(), { 0.85f, 0.65f }, { 0.2f, 0.08f }, goal_message, UIConstants::SubTitleTextPointSize,
		{ UIColor::White }, { 0,  (unsigned int)goal_message.length() }, UITextJustification::CenterLeft);

	m_uiManager.addElement<TextOverlay>(level, L"Game Level Message");
	m_uiManager.addElement<TextOverlay>(threshold, L"Game Threshold Message");
	m_uiManager.addElement<TextOverlay>(streak, L"Game Streak Message");
	m_uiManager.addElement<TextOverlay>(swing, L"Game Swing Message");
	m_uiManager.addElement<TextOverlay>(goal, L"Game Goal Message");

	std::wstring threshold_options = L"+/- 1\n+/- 5\n+ 3\n- 2\n- 5";
	DropDownMenu threshold_selection(m_uiManager.getScreenSize(), { 0.85f, 0.425f }, { 0.2f, 0.08f }, threshold_options, 0.5f, 2);

	std::wstring target_options = L"-10 deg.\n-5 deg.\n0 deg.\n2 deg.\n4 deg.";
	DropDownMenu target_selection(m_uiManager.getScreenSize(), { 0.85f, 0.525f }, { 0.2f, 0.08f }, target_options, 0.5f, 2);

	m_uiManager.addElement<DropDownMenu>(threshold_selection, L"Dropdown Menu 2");
	m_uiManager.addElement<DropDownMenu>(target_selection, L"Dropdown Menu 3");

	//Initialize variables that are specific to this training module
	m_modeType = 0; //represents training mode
	switchMode(); //will load training mode by default

	//Set the upper and lower limits for the goal angle. The goal is randomized after each swing
	//so we want to make sure nothing unrealistic pops up like 90 degrees
	m_goalLimits = { -10.0f, 10.0f };
}

void SwingPathTrainingMode::switchMode()
{
	//This method swaps out necessary UI elements and resets variables
	//as necessary when switching from training mode to game mode.
	if (m_modeType == 0)
	{
		//This represents training mode

		//Make the game mode elements invisible
		m_uiManager.getElement<TextOverlay>(L"Game Level Message")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<TextOverlay>(L"Game Threshold Message")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<TextOverlay>(L"Game Streak Message")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<TextOverlay>(L"Game Swing Message")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<TextOverlay>(L"Game Goal Message")->updateState(UIElementState::Invisible);

		//Initialize the graph with the current value from the goal drop down menu
		std::wstring goal_angle_text = m_uiManager.getElement<DropDownMenu>(L"Dropdown Menu 3")->getSelectedOption();
		goal_angle_text = goal_angle_text.substr(0, goal_angle_text.find(L" ")); //the actual number is everything from the beginning to the first space
		wchar_t* endString;
		m_goalAngle = std::wcstol(&goal_angle_text[0], &endString, 10); //convert the wide string representation of the address to an uint64_t
		 
		initializeGraph();

		//And make training mode elements visible
		m_uiManager.getElement<DropDownMenu>(L"Dropdown Menu 2")->removeState(UIElementState::Invisible);
		m_uiManager.getElement<DropDownMenu>(L"Dropdown Menu 3")->removeState(UIElementState::Invisible);
	}
	else
	{
		//This represents game mode, reset the game variables and display the appropriate UI Elements
		m_currentLevel = 0;
		m_currentStreak = 0;
		createGoalAngle(); //come up with a target goal to swing at
		initializeGraph();
		levelUp(); //leveling up will increase the level to 1 and set the goal threshold appropriately

		//Make the game mode elements visible
		m_uiManager.getElement<TextOverlay>(L"Game Level Message")->removeState(UIElementState::Invisible);
		m_uiManager.getElement<TextOverlay>(L"Game Threshold Message")->removeState(UIElementState::Invisible);
		m_uiManager.getElement<TextOverlay>(L"Game Streak Message")->removeState(UIElementState::Invisible);
		m_uiManager.getElement<TextOverlay>(L"Game Swing Message")->removeState(UIElementState::Invisible);
		m_uiManager.getElement<TextOverlay>(L"Game Goal Message")->removeState(UIElementState::Invisible);

		//And make training mode elements invisible
		m_uiManager.getElement<DropDownMenu>(L"Dropdown Menu 2")->updateState(UIElementState::Invisible);
		m_uiManager.getElement<DropDownMenu>(L"Dropdown Menu 3")->updateState(UIElementState::Invisible);

		//With the game messages visible, make sure they hold the correct values
		updateGameText();
	}
}

void SwingPathTrainingMode::levelUp()
{
	//When leveling up the acceptable threshold for error drops, making it harder to move on
	m_currentLevel++;
	m_toLevelUp = 5; //it takes 5 successful swings to level up

	if (m_currentLevel > 5) m_currentLevel = 5; //for right now 5 is the highest level

	switch (m_currentLevel)
	{
	case 1:
		m_threshold = { -10.0f, 10.0f };
		break;
	case 2:
		m_threshold = { -7.5f, 7.5f };
		break;
	case 3:
		m_threshold = { -5.0f, 5.0f };
		break;
	case 4:
		m_threshold = { -2.5f, 2.5f };
		break;
	default:
	case 5:
		m_threshold = { -1.0f, 1.0f };
		break;
	}
}

void SwingPathTrainingMode::updateGameText()
{
	//Updates the game text to reflect the current state of the game variables
	m_uiManager.getElement<TextOverlay>(L"Game Level Message")->updateText(L"Current Level: " + std::to_wstring(m_currentLevel));
	m_uiManager.getElement<TextOverlay>(L"Game Threshold Message")->updateText(L"Degree Threshold: +/- " + std::to_wstring(m_threshold.second));
	m_uiManager.getElement<TextOverlay>(L"Game Streak Message")->updateText(L"Current Streak: " + std::to_wstring(m_currentStreak));
	m_uiManager.getElement<TextOverlay>(L"Game Swing Message")->updateText(L"Swings to Next Level: " + std::to_wstring(m_toLevelUp));
	m_uiManager.getElement<TextOverlay>(L"Game Goal Message")->updateText(L"Current Goal: " + std::to_wstring(m_goalAngle) + L" deg.");
}

void SwingPathTrainingMode::createGoalAngle()
{
	//A short function for coming up with a random goal angle
	SwingPathTrainingMode::m_goalAngle = (rand() % 101) / 10.0f; //generate a random goal angle between 0.0 and 10.0 degrees
	if (rand() % 100 >= 50) m_goalAngle *= -1; //50-50 chance for a positive or negative angle
}

void SwingPathTrainingMode::initializeGraph()
{
	//This method erases everything currently on the graph, draws new 0 lines,
	//and then draws a green line representing the target line to swing down.
	m_uiManager.getElement<Graph>(L"Graph")->removeAllLines();
	m_uiManager.getElement<Graph>(L"Graph")->addAxisLine(0, 0.0f);
	m_uiManager.getElement<Graph>(L"Graph")->addAxisLine(1, 0.0f);

	float goal_slope = tan(m_goalAngle * DEGREES_TO_RADIANS);

	//Draw a green line representing the goal target line, and a black line representing the golf ball
	std::vector<DirectX::XMFLOAT2> data1, data2;
	for (float i = -0.5f; i <= 0.5f; i += 0.01f) data1.push_back({ i, i * goal_slope });
	for (float i = -PI; i <= PI; i += PI / 50.0f) data2.push_back({ sin(i) / 10.0f, cos(i) / 10.0f });

	m_uiManager.getElement<Graph>(L"Graph")->addGraphData(data2, UIColor::Black, L"Golf Ball");
	m_uiManager.getElement<Graph>(L"Graph")->addGraphData(data1, UIColor::Green, L"Goal Line");
}

void SwingPathTrainingMode::pc_ModeChange(PersonalCaddiePowerMode newMode)
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

void SwingPathTrainingMode::addQuaternions(std::vector<glm::quat> const& quaternions, int quaternion_number, float time_stamp, float delta_t)
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

	m_newQuaternions = true; //this variable is used for adding rotation quaternions to current swing path only a single time
	data_start_timer = std::chrono::steady_clock::now(); //set relative time

	if (!m_converged)
	{
		//if the filter hasn't yet converged add the first quaternion from this set to the convergence array
		//and call the conergenceCheck() method
		m_convergenceQuaternions.push_back(m_quaternions[0]);
		convergenceCheck();
	}
}

void SwingPathTrainingMode::addData(std::vector<std::vector<std::vector<float> > > const& sensorData, float sensorODR, float timeStamp, int totalSamples)
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

void SwingPathTrainingMode::update()
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

	//After all graphical updates are complete, update teh state of the golf swing
	swingUpdate();
}

void SwingPathTrainingMode::handleKeyPress(winrt::Windows::System::VirtualKey pressedKey)
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
		ModeType newMode = ModeType::TRAINING_MENU;
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

void SwingPathTrainingMode::getIMUHeadingOffset(glm::quat heading)
{
	//This method gets the heading offset saved in the IMU class and updates
	//the local heading offset variable with it. This variable is used to make
	//sure the rendered image aligns with the computer monitor
	m_headingOffset = heading;
}

void SwingPathTrainingMode::setCurrentHeadingOffset()
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

void SwingPathTrainingMode::convergenceCheck()
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

void SwingPathTrainingMode::uiElementStateChangeHandler(std::shared_ptr<ManagedUIElement> element)
{
	if (element->name == L"Dropdown Menu 1")
	{
		//This is the drop down menu used for switching between training and game mode. If a new option
		//has been selected, update variables and swap UI Elements as necessary.
		auto dropdown_menu = (DropDownMenu*)(element->element.get());
		if (!(dropdown_menu->selectionInProcess()))
		{
			auto new_mode = dropdown_menu->getSelectedOption();
			if (new_mode == L"Training Mode" && m_modeType != 0)
			{
				//Training mode has been selected and we're currently in game mode
				m_modeType = 0;
				switchMode();
			}
			else if (new_mode == L"Game Mode" && m_modeType != 1)
			{
				m_modeType = 1;
				switchMode();
			}
		}
	}
	else if (element->name == L"Dropdown Menu 2")
	{
		//This is the drop down menu used for changing the target threshold in training mode.
		auto dropdown_menu = (DropDownMenu*)(element->element.get());
		if (!(dropdown_menu->selectionInProcess()))
		{
			//Simply update the values of m_threshold. The angle threshold
			//can be written as a +/- angle, only a + angle or only a - angle
			std::wstring threshold_text = dropdown_menu->getSelectedOption();
			std::wstring threshold_polarity = threshold_text.substr(0, 2);
			std::wstring threshold_value_text = threshold_text.substr(threshold_text.find(L" ") + 1);
			wchar_t* endString;
			float threshold_value = std::wcstof(&threshold_value_text[0], &endString);

			if (threshold_polarity == L"+/") m_threshold = { -threshold_value, threshold_value };
			else if (threshold_polarity == L"+ ") m_threshold = { 0.0f, threshold_value };
			else if (threshold_polarity == L"- ") m_threshold = { -threshold_value, 0.0f };
		}
	}
	else if (element->name == L"Dropdown Menu 3")
	{
		//This is the drop down menu used for changing the target angle in training mode.
		auto dropdown_menu = (DropDownMenu*)(element->element.get());
		if (!(dropdown_menu->selectionInProcess()))
		{
			//Update the m_goalAngle field and update the graph with the current value from the goal drop down menu
			std::wstring goal_angle_text = dropdown_menu->getSelectedOption();
			goal_angle_text = goal_angle_text.substr(0, goal_angle_text.find(L" ")); //the actual number is everything from the beginning to the first space
			wchar_t* endString;
			m_goalAngle = std::wcstof(&goal_angle_text[0], &endString);

			initializeGraph();
		}
	}
}

void SwingPathTrainingMode::preAddressAction()
{
	//Once the old swing data is cleared out, update any game variables that need it
	if (m_modeType == 1)
	{
		//Come up with a new goal angle
		createGoalAngle();

		//Update the game text to reflect the change in game variables since last swing
		updateGameText();
	}

	initializeGraph();

	//When the golfer has successfully come to address, draw a red circle to let them know it's
	//ok to swing.
	Ellipse address_ellipse(m_uiManager.getScreenSize(), { 0.5f, 0.25f }, { MAX_SCREEN_HEIGHT / MAX_SCREEN_WIDTH * 0.033f, 0.033f }, false, UIColor::Red);
	m_uiManager.addElement<Ellipse>(address_ellipse, L"Ellipse 1");
}

void SwingPathTrainingMode::impactAction()
{
	//Whenever we get a new set of quaternions we extract info from them about the
	//club right before impact.

	//First, save information about the rotation quaternions rate of change along the
	//XY plane. This will let us know if the club is swinging down the target line,
	//on an out-to-in path or on an in-to-out path which has large implications for 
	//the flight of the golf ball. Shift data points so that the golf ball will be at 
	//the center of the graph
	for (int i = 0; i < m_quaternions.size(); i++)
	{
		std::vector<float> club_orientation = { 1.0f, 0.0f, 0.0f }; //will get rotated according to the current quaternion
		detectFollowThrough(m_ball_location, club_orientation, QuaternionMultiply(*p_headingOffset, p_quaternions->at(i))); //redundant as this is already checked in GolfSwing class, but leave for now
		m_swingPath.push_back({ club_orientation[1] - m_ball_location[1], club_orientation[0] - m_ball_location[0] });
		m_tangential_swing_speed += m_angularVelocities[i].first;
		m_radial_swing_speed += m_angularVelocities[i].second;
	}
}

void SwingPathTrainingMode::preSwingEndAction()
{
	//Add the actual swing path to the graph
	m_uiManager.getElement<Graph>(L"Graph")->addGraphData(m_swingPath, UIColor::Red);

	//Remove the red 'at-address' ellipse and clear the swing path data for
	//the next swing
	m_uiManager.removeElement<Ellipse>(L"Ellipse 1");
	m_swingPath.clear();
}