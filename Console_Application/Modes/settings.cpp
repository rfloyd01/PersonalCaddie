#include "pch.h"

#include <iostream>
#include <cmath>
#include <string>

#include <Modes/settings.h>

//PUBLIC FUNCTIONS
//Updating and Advancement Functions
void Settings::update()
{
	alertUpdate();
	processInput(); //process FreeSwing specific input first
}
void Settings::processInput()
{
	if (!p_graphics->GetCanPressKey()) return; //only process input if input processing is available

	if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		modeEnd();
		p_graphics->setCurrentMode(ModeType::MAIN_MENU); //set the new mode
		p_graphics->resetKeyTimer();
	}
	else if (glfwGetKey(p_graphics->GetWindow(), GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		p_graphics->resetKeyTimer();

		uint8_t bytes[20] = { 0 };
		std::string line;

		std::cout << "\nPlease enter a byte array in the form: xx xx xx xx ..." << std::endl;
		std::cout << "(*note) leadings zeros must be included, 5 isn't valide but 05 is for example." << std::endl;

		std::getline(std::cin, line);

		for (int i = 0; i < line.size();)
		{
			int first_bits = convertCharToInt(line[i]);
			int second_bits = convertCharToInt(line[i + 1]);
			
			if ((first_bits < 0) || (second_bits < 0))
			{
				std::cout << line[i] << line[i + 1] << " isn't a valid hexadecimal value. Go back to graphic screen" <<
					" and hit enter to try again." << std::endl;
				return;
			}

			bytes[i / 3] = (((first_bits & 0xF) << 4) | (second_bits & 0XF));
			i += 3;
		}

		std::cout << "The first four bytes entered in the array are: " << std::hex << (int)bytes[0] << ", " << (int)bytes[1] << ", " << (int)bytes[2] << ", " << (int)bytes[3] << std::endl;
		
	}
}
void Settings::modeStart()
{
	//initialize text to render
	initializeText();
}
void Settings::modeEnd()
{
	//clear out all text to free up space
	clearAllText();

	//clear data from model map to free up space
	clearAllImages();
}

//PRIVATE FUNCTIONS
//Setup Functions
void Settings::initializeText()
{
	addText(MessageType::TITLE, { "Sensor Settings", 230.0, 550.0, 1.0, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::SUB_TITLE, { "(Press the Enter key to use the console to update settings on the sensor)", 70.0, 520.0, .5, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::FOOT_NOTE, { "Press Esc. to return to Main Menu.", 560.0, 10.0, .33, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });

	addText(MessageType::BODY, { "1. Swing Plane", 20.0, 380.0, 0.75, glm::vec3(0.976, 0.902, 0.475), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "2. Clubface Squareness", 20.0, 335.0, 0.75, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "3. Clubface Tilt", 20.0, 290.0, 0.75, glm::vec3(0.835, 0.518, 0.235), p_graphics->getScreenWidth() });
	addText(MessageType::BODY, { "4. Distance", 20.0, 245.0, 0.75, glm::vec3(1.0, 1.0, 1.0), p_graphics->getScreenWidth() });
}

int Settings::convertCharToInt(char c)
{
	//c can be either a number, an uppercase hexadecimal letter or a lowercase hexadecimal letter. If it's anything else
	//an error should be kicked up.

	//For reference
	//a = 97, f = 102
	//A = 65, F = 70
	//0 = 48, 9 = 57

	if (c < 48) return -1; //not a hexadecimal letter or number
	else if (c <= 57) return c - '0'; //this is a number 0-9
	else if (c < 65) return -1; //not a hexadecimal letter or number
	else if (c <= 70) return c - 'A' + 10; //this is an uppercase hexadecimal letter
	else if (c < 97) return -1; //not a hexadecimal letter or number
	else if (c <= 102) return c - 'a' + 10; //this is a lowercase hexadecimal letter
	else return -1; //anot a hexadecimal letter or number
}