/*
 * Message.h
 * ---------
 * Purpose: Various functions for processing song messages (allocating, reading from file...)
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include <string>

class FileReader;

//====================================
class SongMessage : public std::string
//====================================
{
public:

	typedef void (*ConverterFunc)(char &);

	// Line ending types (for reading song messages from module files)
	enum LineEnding
	{
		leCR,			// Carriage Return (0x0D, \r)
		leLF,			// Line Feed (0x0A \n)
		leCRLF,			// Carriage Return, Line Feed (0x0D0A, \r\n)
		leMixed,		// It is not defined whether Carriage Return or Line Feed is the actual line ending. Both are accepted.
		leAutodetect,	// Detect suitable line ending
	};

	enum
	{
		InternalLineEnding	= '\r',	// The character that represents line endings internally
	};

	// Read song message from a mapped file.
	// [in]  data: pointer to the data in memory that is going to be read
	// [in]  length: number of characters that should be read, not including a possible trailing null terminator (it is automatically appended).
	// [in]  lineEnding: line ending formatting of the text in memory.
	// [in]  pTextConverter: Pointer to a callback function which can be used to pre-process the read characters, if necessary (nullptr otherwise).
	// [out] returns true on success.
	bool Read(const void *data, const size_t length, LineEnding lineEnding, ConverterFunc pTextConverter = nullptr);
	bool Read(FileReader &file, const size_t length, LineEnding lineEnding, ConverterFunc pTextConverter = nullptr);

	// Read comments with fixed line length from a mapped file.
	// [in]  data: pointer to the data in memory that is going to be read
	// [in]  length: number of characters that should be read, not including a possible trailing null terminator (it is automatically appended).
	// [in]  lineLength: The fixed length of a line.
	// [in]  lineEndingLength: The padding space between two fixed lines. (there could for example be a null char after every line)
	// [in]  pTextConverter: Pointer to a callback function which can be used to pre-process the read characters, if necessary (nullptr otherwise).
	// [out] returns true on success.
	bool ReadFixedLineLength(const void *data, const size_t length, const size_t lineLength, const size_t lineEndingLength, ConverterFunc pTextConverter = nullptr);
	bool ReadFixedLineLength(FileReader &file, const size_t length, const size_t lineLength, const size_t lineEndingLength, ConverterFunc pTextConverter = nullptr);

	// Retrieve song message.
	// [in]  lineEnding: line ending formatting of the text in memory.
	// [in]  pTextConverter: Pointer to a callback function which can be used to post-process the written characters, if necessary (nullptr otherwise).
	// [out] returns formatted song message.
	std::string GetFormatted(const LineEnding lineEnding, ConverterFunc pTextConverter = nullptr) const;
};
