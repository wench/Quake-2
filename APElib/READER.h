#include <stdint.h>
#include <tuple> 
#pragma once
class READER
{
	bool readbig;
public:
	//bIIG eNDIAN OR LITTLE eNDIAN PROCESSING
	__declspec(property(put = setbig, get = isbig)) bool BigEndian;
	bool isbig() { return readbig; }
	void setbig(bool newbig) { readbig = newbig; }

	__declspec(property(get = size)) size_t Length;
	virtual size_t size() = 0;
	virtual size_t curpos() = 0;
	virtual uint8_t readu8() = 0;
	__declspec(property(get = Get_remaining)) size_t remaining;
	virtual size_t Get_remaining() { return size() - curpos(); }
	int8_t readi8() {
		return readu8();
	}
	uint16_t readu16() {
		uint16_t b1 = readu8();
		uint16_t b2 = readu8();
		if (readbig)
			return b2 | b1 << 8;
		else
			return b1 | b2 << 8;
	}
	int16_t readi16() {
		return readu16();
	}
	uint32_t readu32() {
		uint32_t w1 = readu16();
		uint32_t w2 = readu16();
		if (readbig)
			return w2 | w1 << 16;
		else
			return w1 | w2 << 16;
	}
	float readf32() {
		auto i = readu32();
		return *(float*)& i;
	}
	int32_t readi32() {
		return readu32();
	}
	uint64_t readu64() {
		uint64_t i1 = readu32();
		uint64_t i2 = readu32();
		if (readbig)
			return i2 | i1 << 32;
		else
			return i1 | i2 << 32;

	}
	int64_t readi64() {
		return readu64();
	}
	double readf64() {
		auto i = readu64();
		return *(double*)& i;
	}
public:
	template<typename T> T readT();
	template<typename T> T& read(T& v)
	{
		v = readT<T>();
		return v;
	}

	virtual size_t read(size_t bytes,  char* buf)
	{
		auto rem = remaining;
		if (rem < bytes)
			bytes = rem;

		for (size_t b = 0; b < bytes; b++)
			buf[b] = readu8();

		return bytes;
	}

	template<class Type1, class... Types>
	std::tuple<Type1, Types...> readTuple()
	{
		std::tuple<Type1, Types...> t;
		readTuple(t);
		return t;
	}
	template<class Type1, class... Types>
	void readTuple(std::tuple<Type1, Types...> &res)
	{
		Type1 v1;
		read(v1);
		auto t1 = std::make_tuple(v1);
		std::tuple<Types...> t2;
		readTuple(t2);
		res =  std::tuple_cat(t1, t2);
	}

	 template<class Type1>
	  void readTuple(std::tuple<Type1 > &res)
	 {
		 Type1 v1;
		 read(v1);
		 res = std::make_tuple(v1);
	 }



template<> int32_t readT<int32_t>()
{
	return readi32();
}
template<> long readT<long>()
{
	return readi32();
}
template<> uint32_t readT<uint32_t>()
{
	return readu32();
}
template<> unsigned long readT<unsigned long>()
{
	return readu32();
}
template<> int16_t readT<int16_t>()
{
	return readi16();
}
template<> uint16_t readT<uint16_t>()
{
	return readu16();
}
template<> int8_t readT<int8_t>()
{
	return readi8();
}
template<> uint8_t readT<uint8_t>()
{
	return readu8();
}

template<> int64_t readT<int64_t>()
{
	return readi64();
}
template<> uint64_t readT<uint64_t>()
{
	return readu64();
}
template<> float readT<float>()
{
	return readf32();
}
template<> double readT<double>()
{
	return readf64();
}
};