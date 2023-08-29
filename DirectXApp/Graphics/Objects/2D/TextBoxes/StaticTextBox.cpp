#include "pch.h"
#include "StaticTextBox.h"

StaticTextBox::StaticTextBox(DirectX::XMFLOAT2 location, DirectX::XMFLOAT2 size, std::wstring text)
{
	m_location = location;
	m_size = size;

	//The StaticTextBox is the most basic of UI Elements. It's just a white rectangle that has
	//black text overlayed on top of it. If there's too much text then it gets clipped at the
	//bottom of the text box.

	addText(text);
}

void StaticTextBox::addText(std::wstring text)
{
	//The text passed in get's converted into a Text class object and is added 
	//to the end of the elementText vector. Added text will just be simple, black text.
	UIText newText(text, { {0.0, 0.0, 0.0} }, {0, text.length()}, UITextType::ELEMENT_TEXT);
	m_elementText.push_back(newText);
}