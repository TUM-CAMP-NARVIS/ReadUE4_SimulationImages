// ReadFromFloat16.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <stdint.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#if __cplusplus < 201703L // If the version of C++ is less than 17
	// It was still in the experimental:: namespace
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

// Hardcoded. Maybe to a meta file instead?

const int height = 1024;
const int width = 1024;

std::unique_ptr<char[]> readFile(std::string filename, int32_t& _fileSize)
{
	// open the file:
	std::ifstream file(filename, std::ios::binary);
	file.unsetf(std::ios::skipws);

	// get its size:
	int32_t fileSize;

	file.seekg(0, std::ios::end);
	fileSize = static_cast<int32_t>(file.tellg());
	file.seekg(0, std::ios::beg);
	
	std::vector<float> test;
	test.resize(fileSize / sizeof(float));

	std::unique_ptr<char[]> buffer(new char[fileSize]);

	// read the data:
	file.read(buffer.get(), fileSize);

	file.close();
	_fileSize = fileSize;

	return buffer;
}

void showRawFloat(std::string file)
{
	std::vector<float> fImageValues;
	int32_t fileSize;
	std::unique_ptr<char[]> buffer = readFile(file, fileSize);

	fImageValues.resize(fileSize / 4);

	for (int64_t index = 0; index < fImageValues.size(); ++index)
	{
		fImageValues[index] = *reinterpret_cast<float*>(&buffer[index * 4]);
	}

	std::unique_ptr<char[]> cvDataGrey(new char[fImageValues.size()]);

	float maxD = fImageValues[0];
	float minD = fImageValues[0];

	for (int index = 0; index < fImageValues.size(); index++)
	{
		maxD = std::max(maxD, fImageValues[index]);
		minD = std::min(minD, fImageValues[index]);
	}

	maxD = 400.0f;

	for (int index = 0; index < fImageValues.size(); index++)
	{
		if (fImageValues[index] > minD && fImageValues[index] < maxD)
		{
			cvDataGrey[index] = static_cast<char>((fImageValues[index] - minD) * 255 / (maxD - minD));
		}
		else if (fImageValues[index] >= maxD)
		{
			cvDataGrey[index] = 255;
		}
		else
		{
			cvDataGrey[index] = 0;
		}
	}

	cv::Mat imgGrey(height, width, CV_8UC1, cvDataGrey.get());

	cv::imshow("Depth", imgGrey);
}

void showRawUINT8(std::string file)
{
	int32_t fileSize;
	std::unique_ptr<char[]> buffer = readFile(file, fileSize);

	cv::Mat img(height, width, CV_8UC4, buffer.get());

	cv::imshow("Color", img);
}

int main(int argc, void** argv)
{
	if (argc < 2)
	{
		return -1;
	}

	std::string path = std::string(reinterpret_cast<char*>(argv[1]));
	std::vector<std::string> allFiles;

	for (const auto & entry : fs::directory_iterator(path))
	{
		allFiles.push_back(entry.path().u8string());
	}

	for (auto file : allFiles)
	{
		if (file.find(".raw16") != std::string::npos)
		{
			showRawFloat(file);
		}

		if (file.find(".raw8") != std::string::npos)
		{
			showRawUINT8(file);
		}
		cv::waitKey(1);
	}

	cv::waitKey(0);

	return 0;
}