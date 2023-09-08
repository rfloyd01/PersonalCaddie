#include "pch.h"
#include "GraphMode.h"

GraphMode::GraphMode()
{
	//set a gray background color for the mode
	m_backgroundColor = UIColor::Blue;
}

uint32_t GraphMode::initializeMode(winrt::Windows::Foundation::Size windowSize, uint32_t initialState)
{
	//Create a button that will generate a sin graph
	TextButton recordButton(windowSize, { 0.1, 0.2 }, { 0.12, 0.1 }, L"Start Recording Data");
	TextButton deleteGraphButton(windowSize, { 0.9, 0.2 }, { 0.12, 0.1 }, L"Delete the graph!");
	Graph graph(windowSize, { 0.5, 0.65 }, { 0.9, 0.6 });

	m_uiElements.push_back(std::make_shared<TextButton>(recordButton));
	m_uiElements.push_back(std::make_shared<TextButton>(deleteGraphButton));
	m_uiElements.push_back(std::make_shared<Graph>(graph));

	//Initialize all overlay text
	initializeTextOverlay(windowSize);

	m_state = 0;
	
	//When this mode is initialzed we go into a state of CanTransfer
	return (ModeState::CanTransfer | ModeState::PersonalCaddieSensorIdleMode);
}

void GraphMode::uninitializeMode()
{
	//The only thing to do when leaving the main menu mode is to clear
	//out all text in the text map and color map
	for (int i = 0; i < m_uiElements.size(); i++) m_uiElements[i] = nullptr;
	m_uiElements.clear();
}

void GraphMode::initializeTextOverlay(winrt::Windows::Foundation::Size windowSize)
{
	//Title information
	std::wstring title_message = L"Graph Mode";
	TextOverlay title(windowSize, { UIConstants::TitleTextLocationX, UIConstants::TitleTextLocationY }, { UIConstants::TitleTextSizeX, UIConstants::TitleTextSizeY },
		title_message, UIConstants::TitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)title_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(title));

	////Sub-Title information
	//std::wstring subtitle_message = L"Record Data from a connected device and display it in graph form";
	//TextOverlay subtitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
	//	subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	//m_uiElements.push_back(std::make_shared<TextOverlay>(subtitle));

	//Footnote information
	std::wstring footnote_message = L"Press Esc. to return to main menu";
	TextOverlay footnote(windowSize, { UIConstants::FootNoteTextLocationX, UIConstants::FootNoteTextLocationY }, { UIConstants::FootNoteTextSizeX, UIConstants::FootNoteTextSizeY },
		footnote_message, UIConstants::FootNoteTextPointSize, { UIColor::White }, { 0,  (unsigned int)footnote_message.length() }, UITextJustification::LowerRight);
	m_uiElements.push_back(std::make_shared<TextOverlay>(footnote));
}

uint32_t GraphMode::handleUIElementStateChange(int i)
{
	if (i == 0)
	{
		/*std::vector<DirectX::XMFLOAT2> data;
		float end = 2 * 3.14159 * m_sinePeaks, increment = end / m_dataPoints;
		for (float i = 0; i <= end; i += increment)
		{
			DirectX::XMFLOAT2 point = { i, sinf(i) };
			data.push_back(point);
		}

		((Graph*)m_uiElements[2].get())->addNewDataPoints(data);

		m_sinePeaks++;
		OutputDebugString(L"Clicked the graph button.\n");*/

		//UI Element 0 is begin/stop recording button. This will bring the personal caddie
		//into or out of sensor active mode. If coming out of active mode it will also display
		//a graph with the data collected.
		if (!(m_state & GraphModeState::RECORDING))
		{
			//We aren't currently recording anything so we put the Personal Caddie into sensor
			//active mode and start listening for data updates.
			m_uiElements[0]->getText()->message = L"Stop Recording Data";

			//Clear out any existing data points
			((Graph*)m_uiElements[2].get())->removeAllLines();
		}
		else
		{
			//We're currently recording data, we stop by alerting the Personal Caddie to enter
			//the sensor idle power mode. We then display any data gethered during the recording
			//session

			m_uiElements[0]->getText()->message = L"Start Recording Data";
		}
		m_state ^= GraphModeState::RECORDING; //toggle the recording state
	}
	else if (i == 1)
	{
		
	}
	return m_state;
}