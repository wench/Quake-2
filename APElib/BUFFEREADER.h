#pragma once
#include "READER.h"
class BUFFEREADER :
	public READER
{
	const uint8_t* buffer;
	size_t bufsize;
	size_t offset;
public:
	BUFFEREADER(const uint8_t* b, size_t size): buffer(b), bufsize(size), offset(0)
	{}
	virtual size_t size()
	{
		
		return bufsize;

	}
	virtual size_t curpos()
	{
		return offset;
	}
	virtual uint8_t readu8()
	{
		if (offset >= bufsize) return -1;
		return buffer[offset++];
	}

	virtual size_t read(size_t bytes,  char* buf)
	{
		auto rem = remaining;
		if (rem < bytes)
			bytes = rem;
		if (bytes <= 0) return 0;

		memcpy(buf, buffer + offset, bytes);
		offset += bytes;

		return bytes;
	}
};

