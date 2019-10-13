#pragma once
#include <cstdio>
#include "READER.h"
class FILEREADER :public READER
{
	std::FILE* file;
	size_t file_size;
public:
	FILEREADER(std::FILE* f):file(f)
	{
		size_t cur = std::ftell(file);
		std::fseek(file, 0, SEEK_END);
		file_size = std::ftell(file);
		std::fseek(file, cur, SEEK_SET);

	 }
	virtual size_t curpos()
	{
		return std::ftell(file);
	}
	virtual size_t size()
	{

		return file_size;

	}
	virtual uint8_t readu8()
	{
		return getc(file);
	}
	virtual size_t read(size_t bytes,  char* buf)
	{
		return fread(buf,1,bytes,file);
	}

};

