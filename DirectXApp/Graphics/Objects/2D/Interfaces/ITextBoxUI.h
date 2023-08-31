#pragma once

//An interface for creating text boxes. Depending on the type
//of text box being made text will be added differently so the
//addText method must be overridden (for example, a static text
//box will have all text added in one shot, while a drop down menu
//will have the input string separated into separate strings and
//each one added as an element of a vector).

struct ITextBoxUI
{
	virtual uint32_t addText(std::wstring text) = 0;
};