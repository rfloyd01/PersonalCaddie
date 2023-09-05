#pragma once

//Some UI Elements need to know the height or width for
//individual lines of text, or a whole block of text. Since
//Direct write automatically wraps text, we can't know how
//large or wide the text will be until the master renderer
//calls the DrawTextLayout() method on the text in question.
//Any UI Element that implements this interface will override
//the getTextDimensions method as a means of figuring out what
//specific text dimension(s) is needed.

struct ITextDimensionsUI
{
	virtual std::vector<UIText*> setTextDimension() = 0;
	virtual void repositionText() = 0;
};