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
	TextButton graphButton(windowSize, { 0.1, 0.2 }, { 0.12, 0.1 }, L"Create a graph!");
	Graph graph(windowSize, { 0.5, 0.65 }, { 0.9, 0.6 });

	m_uiElements.push_back(std::make_shared<TextButton>(graphButton));
	m_uiElements.push_back(std::make_shared<Graph>(graph));

	//Initialize all overlay text
	initializeTextOverlay(windowSize);
	
	//When this mode is initialzed we go into a state of CanTransfer
	return ModeState::CanTransfer;
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

	//Sub-Title information
	std::wstring subtitle_message = L"Record Data from a connected device and display it in graph form";
	TextOverlay subtitle(windowSize, { UIConstants::SubTitleTextLocationX, UIConstants::SubTitleTextLocationY }, { UIConstants::SubTitleTextSizeX, UIConstants::SubTitleTextSizeY },
		subtitle_message, UIConstants::SubTitleTextPointSize, { UIColor::White }, { 0,  (unsigned int)subtitle_message.length() }, UITextJustification::CenterCenter);
	m_uiElements.push_back(std::make_shared<TextOverlay>(subtitle));

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
		std::vector<DirectX::XMFLOAT2> data;
		float end = 2 * 3.14159 * m_sinePeaks, increment = end / m_dataPoints;
		for (float i = 0; i <= end; i += increment)
		{
			DirectX::XMFLOAT2 point = { i, sinf(i) };
			data.push_back(point);
		}

		((Graph*)m_uiElements[1].get())->addNewDataPoints(data);

		m_sinePeaks++;
		OutputDebugString(L"Clicked the graph button.\n");
	}
	return 0;
}