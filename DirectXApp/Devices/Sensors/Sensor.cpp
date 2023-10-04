#include "pch.h"

#include "Sensor.h"

#include <algorithm>
#include <iostream>
#include <fstream>

//using namespace winrt;
//using namespace Windows::Foundation;

Sensor::Sensor()
{
	//Any kind of initialization that's common regardless of sensor type will go here
}

std::pair<const float*, const float**> Sensor::getCalibrationNumbers()
{
	//we pass const pointers to the calibration data because we don't want other classes
	//to be able to directly change the calibration values
	return { this->calibration_offsets, this->calibration_gains };
};

void Sensor::setCalibrationNumbers(float* offset, float** gain)
{

}

void Sensor::getCalibrationNumbersFromTextFile()
{
	//Reads the calibration data from the appropriate text file. If the file doesn't exist
	//a new one will be created with default calibration numbers
	winrt::Windows::Storage::StorageFolder localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalCacheFolder();
	auto getCalibrationFolder = localFolder.GetFolderAsync(L"Calibration");

	getCalibrationFolder.Completed([localFolder, this](
		winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFolder> const& sender,
		winrt::Windows::Foundation::AsyncStatus const asyncStatus)
		{
			if (asyncStatus != winrt::Windows::Foundation::AsyncStatus::Error)
			{
				//The folder exists, attempt to get the correct calibration file from it
				auto getCalibrationFile = sender.get().GetFileAsync(this->calibrationFile);
				getCalibrationFile.Completed([localFolder, this](
					winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFile> const& sender,
					winrt::Windows::Foundation::AsyncStatus const asyncStatus)
					{
						if (asyncStatus != winrt::Windows::Foundation::AsyncStatus::Error)
						{
							//write the new address to the file
							auto getCalText = winrt::Windows::Storage::FileIO::ReadTextAsync(sender.get());
							getCalText.Completed([this](
								winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> const& sender,
								winrt::Windows::Foundation::AsyncStatus const asyncStatus)
								{
									if (asyncStatus != winrt::Windows::Foundation::AsyncStatus::Error)
									{
										//convert the text into something meaningful
										convertTextToCalNumbers(sender.get());
									}
									else
									{
										OutputDebugString(L"An error occured when trying to read the Calibration file\n");
									}
								});
						}
						else
						{
							//The calibration file doesn't exist yet, create it now and fill it with default values
							setCalibrationNumbersInTextFile();
						}
					});
			}
			else
			{
				//The calibration folder doesn't exist yet, create it now
				auto createFolder = localFolder.CreateFolderAsync(L"Calibration");
				createFolder.Completed([this](
					winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFolder> const& sender,
					winrt::Windows::Foundation::AsyncStatus const asyncStatus)
					{
						if (asyncStatus != winrt::Windows::Foundation::AsyncStatus::Error)
						{
							//after successfully creating the folder, recursively call the getCalibrationNumbers() method again
							getCalibrationNumbersFromTextFile();
						}
						else
						{
							OutputDebugString(L"An error occured when trying to create the Calibration folder,\n");
						}
					});
			}
		});
}

void Sensor::setCalibrationNumbersInTextFile()
{
	//will set the calibration numbers for the particular sensor
	winrt::Windows::Storage::StorageFolder localFolder = winrt::Windows::Storage::ApplicationData::Current().LocalCacheFolder();
	auto getCalibrationFolder = localFolder.GetFolderAsync(L"Calibration");

	getCalibrationFolder.Completed([localFolder, this](
		winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFolder> const& sender,
		winrt::Windows::Foundation::AsyncStatus const asyncStatus)
		{
			if (asyncStatus != winrt::Windows::Foundation::AsyncStatus::Error)
			{
				//The folder exists, attempt to get the correct calibration file from it
				auto getCalibrationFile = sender.get().GetFileAsync(this->calibrationFile);
				getCalibrationFile.Completed([sender, this](
					winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFile> const& sender_two,
					winrt::Windows::Foundation::AsyncStatus const asyncStatus)
					{
						if (asyncStatus != winrt::Windows::Foundation::AsyncStatus::Error)
						{
							//write the new address to the file
							winrt::Windows::Storage::FileIO::WriteTextAsync(sender_two.get(), convertCalNumbersToText());
						}
						else
						{
							//The calibration file doesn't exist yet, create it now and fill it with default values
							auto createFile = sender.get().CreateFileAsync(calibrationFile);
							createFile.Completed([this](
								winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFile> const& sender,
								winrt::Windows::Foundation::AsyncStatus const asyncStatus)
								{
									if (asyncStatus != winrt::Windows::Foundation::AsyncStatus::Error)
									{
										//The file was successfully created, recursively call the setCalibrationNumbers() method
										setCalibrationNumbersInTextFile();
									}
									else
									{
										OutputDebugString(L"An error occured when trying to create the Calibration file\n");
									}
								});
						}
					});
			}
			else
			{
				//The calibration folder doesn't exist yet, create it now
				auto createFolder = localFolder.CreateFolderAsync(L"Calibration");
				createFolder.Completed([this](
					winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFolder> const& sender,
					winrt::Windows::Foundation::AsyncStatus const asyncStatus)
					{
						if (asyncStatus != winrt::Windows::Foundation::AsyncStatus::Error)
						{
							//after successfully creating the folder, recursively call the setCalibrationNumbers() method again
							setCalibrationNumbersInTextFile();
						}
						else
						{
							OutputDebugString(L"An error occured when trying to create the Calibration folder,\n");
						}
					});
			}
		});
}

float Sensor::getConversionRate() { return this->conversion_rate; }
float Sensor::getCurrentODR() { return this->current_odr; }

void Sensor::convertTextToCalNumbers(winrt::hstring calInfo)
{
	//Convert the hstring to a wstring for ease of manipulation
	std::wstring calText = calInfo.c_str();
	
	//Split the string up by newline characters
	int i = 0, j = 0, readLines = 0;
	float* current_array = calibration_offsets;

	while (j != std::string::npos)
	{
		j = calText.find(L'\n', i);

		std::wstring textLine = calText.substr(i, j - i);
		i = j + 1;

		if (textLine != L"")
		{
			current_array[(readLines++) % 3] = std::stof(textLine);
			if (readLines == 3) current_array = calibration_gain_x;
			else if (readLines == 6) current_array = calibration_gain_y;
			else if (readLines == 9) current_array = calibration_gain_z;
		}
	}
}