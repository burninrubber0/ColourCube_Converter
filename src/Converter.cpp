#include <Converter.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include <binaryio/binaryreader.hpp>
#include <binaryio/binarywriter.hpp>

int Converter::run(int argc, char* argv[])
{
	if (getArgs(argc, argv))
		return 1;

	return convert();
}

void Converter::showHelp()
{
	std::cout << "ColourCube Converter v1.0 by burninrubber0\n"
		<< "Converts Xbox 360 ColourCube resources to PC (RGB24).\n"
		<< "Usage: ColourCube_Converter <input> <output>";
}

int Converter::getArgs(int argc, char* argv[])
{
	if (argc != 3)
	{
		showHelp();
		return 1;
	}

	input = argv[1];
	output = argv[2];

	return 0;
}

int Converter::convert()
{
	// Create the input reader
	std::ifstream in(input, std::ios::in | std::ios::binary);
	if (in.fail())
		return 1;

	size_t inputSize = std::filesystem::file_size(input);
	const auto& buffer = std::make_shared<std::vector<uint8_t>>(inputSize);
	in.read(reinterpret_cast<char*>(buffer->data()), inputSize);
	binaryio::BinaryReader reader(buffer);
	reader.Set64BitMode(false); // TODO: PS4 probably has the offset as 64-bit
	reader.SetBigEndian(true); // All files are effectively big endian

	in.close();

	// Read the header
	reader.Seek(0);
	uint32_t size = reader.Read<uint32_t>(); // Num textures or length/width
	uint32_t dataOffset = reader.Read<uint32_t>();

	// Read the data and convert it to standard RGB24
	// One array of 24 ints is a single column (or row?) of 32 pixels
	auto rawData = std::make_unique<std::array<std::array<std::array<int32_t, 24>, 32>, 32>>();

	// Convert the pixel arrays to RGB24
	reader.Seek(dataOffset);
	// For each of the 32 textures (0xC00)
	for (int i = 0; i < 32; ++i)
	{
		// For each of the 16 2-pixel wide interleaved columns (0xC0)
		for (int j = 0; j < 32; j += 2)
		{
			// Read 4 pixels of column 1, then 4 of column 2 (0xC[2])
			for (int k = 0; k < 24; k += 3)
			{
				rawData->data()[i][j][k] = reader.Read<int32_t>();
				rawData->data()[i][j][k + 1] = reader.Read<int32_t>();
				rawData->data()[i][j][k + 2] = reader.Read<int32_t>();
				rawData->data()[i][j + 1][k] = reader.Read<int32_t>();
				rawData->data()[i][j + 1][k + 1] = reader.Read<int32_t>();
				rawData->data()[i][j + 1][k + 2] = reader.Read<int32_t>();
			}
		}
	}

	// Correct the out-of-order columns
	for (int i = 0; i < 32; ++i) // Each texture
	{
		std::array<std::array<int32_t, 24>, 32> rawDataFixed = rawData->data()[i];
		// Apply this change to the latter quarters of each 0xC00
		for (int j = 16; j < 32; ++j) // Each 0x60 block in the 2nd half
		{
			// Left pixels become right, right pixels become left
			for (int k = 0; k < 12; ++k)
				rawDataFixed.data()[j][k] = rawData->data()[i][j][k + 12];
			for (int k = 12; k < 24; ++k)
				rawDataFixed.data()[j][k] = rawData->data()[i][j][k - 12];
		}
		rawData->data()[i] = rawDataFixed;
	}

	// Correct out-of-order rows
	for (int i = 0; i < 4; ++i) // Pattern occurs 4 times, or 8 counting reverse
	{
		auto rawDataFixed = std::make_unique<std::array<std::array<std::array<int32_t, 24>, 32>, 32>>(*rawData);
		// Copy 0x300-long chunks to the correct positions
		for (int j = 0; j < 8; ++j)
		{
			rawDataFixed->data()[i * 8][j + 8] = rawData->data()[i * 8][j + 16]; // Copy 0x600 to 0x300
			rawDataFixed->data()[i * 8][j + 16] = rawData->data()[i * 8 + 2][j]; // 0x1800 to 0x600
			rawDataFixed->data()[i * 8][j + 24] = rawData->data()[i * 8 + 2][j + 16]; // 0x1E00 to 0x900
			rawDataFixed->data()[i * 8 + 1][j] = rawData->data()[i * 8][j + 8]; // 0x300 to 0xC00
			rawDataFixed->data()[i * 8 + 1][j + 8] = rawData->data()[i * 8][j + 24]; // 0x900 to 0xF00
			rawDataFixed->data()[i * 8 + 1][j + 16] = rawData->data()[i * 8 + 2][j + 8]; // 0x1B00 to 0x1200
			rawDataFixed->data()[i * 8 + 1][j + 24] = rawData->data()[i * 8 + 2][j + 24]; // 0x2100 to 0x1500
			rawDataFixed->data()[i * 8 + 2][j] = rawData->data()[i * 8 + 1][j]; // 0xC00 to 0x1800
			rawDataFixed->data()[i * 8 + 2][j + 8] = rawData->data()[i * 8 + 1][j + 16]; // 0x1200 to 0x1B00
			rawDataFixed->data()[i * 8 + 2][j + 16] = rawData->data()[i * 8 + 3][j]; // 0x2400 to 0x1E00
			rawDataFixed->data()[i * 8 + 2][j + 24] = rawData->data()[i * 8 + 3][j + 16]; // 0x2A00 to 0x2100
			rawDataFixed->data()[i * 8 + 3][j] = rawData->data()[i * 8 + 1][j + 8]; // 0xF00 to 0x2400
			rawDataFixed->data()[i * 8 + 3][j + 8] = rawData->data()[i * 8 + 1][j + 24]; // 0x1500 to 0x2700
			rawDataFixed->data()[i * 8 + 3][j + 16] = rawData->data()[i * 8 + 3][j + 8]; // 0x2700 to 0x2A00
			rawDataFixed->data()[i * 8 + 4][j] = rawData->data()[i * 8 + 4][j + 16]; // 0x3600 to 0x3000
			rawDataFixed->data()[i * 8 + 4][j + 8] = rawData->data()[i * 8 + 4][j]; // 0x3000 to 0x3300
			rawDataFixed->data()[i * 8 + 4][j + 16] = rawData->data()[i * 8 + 6][j + 16]; // 0x4E00 to 0x3600
			rawDataFixed->data()[i * 8 + 4][j + 24] = rawData->data()[i * 8 + 6][j]; // 0x4800 to 0x3900
			rawDataFixed->data()[i * 8 + 5][j] = rawData->data()[i * 8 + 4][j + 24]; // 0x3900 to 0x3C00
			rawDataFixed->data()[i * 8 + 5][j + 8] = rawData->data()[i * 8 + 4][j + 8]; // 0x3300 to 0x3F00
			rawDataFixed->data()[i * 8 + 5][j + 16] = rawData->data()[i * 8 + 6][j + 24]; // 0x5100 to 0x4200
			rawDataFixed->data()[i * 8 + 5][j + 24] = rawData->data()[i * 8 + 6][j + 8]; // 0x4B00 to 0x4500
			rawDataFixed->data()[i * 8 + 6][j] = rawData->data()[i * 8 + 5][j + 16]; // 0x4200 to 0x4800
			rawDataFixed->data()[i * 8 + 6][j + 8] = rawData->data()[i * 8 + 5][j]; // 0x3C00 to 0x4B00
			rawDataFixed->data()[i * 8 + 6][j + 16] = rawData->data()[i * 8 + 7][j + 16]; // 0x5A00 to 0x4E00
			rawDataFixed->data()[i * 8 + 6][j + 24] = rawData->data()[i * 8 + 7][j]; // 0x5400 to 0x5100
			rawDataFixed->data()[i * 8 + 7][j] = rawData->data()[i * 8 + 5][j + 24]; // 0x4500 to 0x5400
			rawDataFixed->data()[i * 8 + 7][j + 8] = rawData->data()[i * 8 + 5][j + 8]; // 0x3F00 to 0x5700
			rawDataFixed->data()[i * 8 + 7][j + 16] = rawData->data()[i * 8 + 7][j + 24]; // 0x5D00 to 0x5A00
			rawDataFixed->data()[i * 8 + 7][j + 24] = rawData->data()[i * 8 + 7][j + 8]; // 0x5700 to 0x5D00
		}
		*rawData = *rawDataFixed;
	}

	// Write to buffer
	binaryio::BinaryWriter writer;
	writer.Write<uint32_t>(size);
	writer.Write<uint32_t>(dataOffset);
	writer.Seek(dataOffset);
	writer.SetBigEndian(true);
	for (int i = 0; i < 32; ++i)
		for (int j = 0; j < 32; ++j)
			for (int k = 0; k < 24; ++k)
				writer.Write<int32_t>(rawData->data()[i][j][k]);

	std::ofstream out(output, std::ios::out | std::ios::binary);
	out << writer.GetStream().rdbuf();
	out.close();

	return 0;
}

Converter::Converter()
{

}