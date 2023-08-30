#include "pch.h"
#include "TextOverlay.h"

TextOverlay::TextOverlay(std::wstring const& text, std::vector<UITextColor> const& colors, std::vector<unsigned long long> const& colorLocations, UITextType textType, winrt::Windows::Foundation::Size windowSize)
{
	//Unlike other text boxes, the TextOverlay box doesn't feature any UI Elements other than pure text. Since there's only text,
	//we just instantiate this class directly with a text. There are a few different classes of overlay text, so depending on the
	//textType variable passed in we will create this class in a certain spot with a certain font size. We can also set the colors
	//if we want, if we don't need anything fancy then the colors will just default to white
	UIText defaultText(text, 0, { 0, 0 }, { 0, 0 }, colors, colorLocations, textType);
	switch (textType)
	{
	case UITextType::TITLE:
	{
		m_location = { UIConstants::TitleRectangleX0, UIConstants::TitleRectangleY0 };
		m_size = { UIConstants::TitleRectangleX1 - UIConstants::TitleRectangleX0, UIConstants::TitleRectangleY1 - UIConstants::TitleRectangleY0 };
		m_fontSize = UIConstants::TitleTextPointSize;
		defaultText.justification = UITextJustification::CenterCenter;
		break;
	}
	case UITextType::SUB_TITLE:
	{
		m_location = { UIConstants::SubTitleRectangleX0, UIConstants::SubTitleRectangleY0 };
		m_size = { UIConstants::SubTitleRectangleX1 - UIConstants::SubTitleRectangleX0, UIConstants::SubTitleRectangleY1 - UIConstants::SubTitleRectangleY0 };
		m_fontSize = UIConstants::SubTitleTextPointSize;
		defaultText.justification = UITextJustification::UpperCenter;
		break;
	}
	case UITextType::BODY:
	{
		m_location = { UIConstants::BodyRectangleX0, UIConstants::BodyRectangleY0 };
		m_size = { UIConstants::BodyRectangleX1 - UIConstants::BodyRectangleX0, UIConstants::BodyRectangleY1 - UIConstants::BodyRectangleY0 };
		m_fontSize = UIConstants::BodyTextPointSize;
		defaultText.justification = UITextJustification::UpperLeft;
		break;
	}
	case UITextType::SENSOR_INFO:
	{
		m_location = { UIConstants::SensorInfoRectangleX0, UIConstants::SensorInfoRectangleY0 };
		m_size = { UIConstants::SensorInfoRectangleX1 - UIConstants::SensorInfoRectangleX0, UIConstants::SensorInfoRectangleY1 - UIConstants::SensorInfoRectangleY0 };
		m_fontSize = UIConstants::SensorInfoTextPointSize;
		defaultText.justification = UITextJustification::LowerRight;
		break;
	}
	case UITextType::ALERT:
	{
		m_location = { UIConstants::AlertRectangleX0, UIConstants::AlertRectangleY0 };
		m_size = { UIConstants::AlertRectangleX1 - UIConstants::AlertRectangleX0, UIConstants::AlertRectangleY1 - UIConstants::AlertRectangleY0 };
		m_fontSize = UIConstants::AlertTextPointSize;
		defaultText.justification = UITextJustification::LowerCenter;
		break;
	}
	case UITextType::FOOT_NOTE:
	{
		m_location = { UIConstants::FootNoteRectangleX0, UIConstants::FootNoteRectangleY0 };
		m_size = { UIConstants::FootNoteRectangleX1 - UIConstants::FootNoteRectangleX0, UIConstants::FootNoteRectangleY1 - UIConstants::FootNoteRectangleY0 };
		m_fontSize = UIConstants::FootNoteTextPointSize;
		defaultText.justification = UITextJustification::LowerRight;
		break;
	}
	}

	m_textOverlay.push_back(defaultText);
	resize(windowSize); //sets the appropriate sizes for both the rectangle and text
}

void TextOverlay::addText(std::wstring text)
{
	//We can only overwrite the message of the current text, the font, colors, etc. are set
	//in the constructor. With that said, only changing the text will make any text colors become
	//messed up potentially, so the TextOverlay class is static. To update an instance of this
	//class it must be deleted and a new one created.
}

void TextOverlay::resize(winrt::Windows::Foundation::Size windowSize)
{
	//All units passed in to the constructor are in window percentages so we
	//convert to actual pixels here.
	m_textOverlay[0].startLocation.x = m_location.x * windowSize.Width;
	m_textOverlay[0].startLocation.y = m_location.y * windowSize.Height;

	m_textOverlay[0].renderArea.x = m_size.x * windowSize.Width;
	m_textOverlay[0].renderArea.y = m_size.y * windowSize.Height;

	m_textOverlay[0].fontSize = m_fontSize * windowSize.Height;
}