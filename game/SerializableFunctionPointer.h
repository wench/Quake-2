#include <cstddef>
#pragma once
class ISerializableFunctionPointer
{
private:
	static ISerializableFunctionPointer* list;
	ISerializableFunctionPointer *next;
protected:
	const char* name;
public:
	ISerializableFunctionPointer(const char* name = nullptr) :name(name)
	{
		if (name)
		{

			next = list;
			list = this;
		}
		else
			next = 0;

	}	

	const char* get_name() { return name; }
	static ISerializableFunctionPointer* GetInterface(const char* name);
	static 	ISerializableFunctionPointer* GetfromFunction(void* funcptr);
//	virtual void inplacecopy(ISerializableFunctionPointer*) = 0;
	virtual bool operator !() = 0;
	virtual  operator bool() = 0;
	virtual void* get_fpointer() = 0;
		
};

template <typename Tret, typename Ta1> class SerializableFunctionPointer1 : public ISerializableFunctionPointer
{
	typedef Tret(*myfptype)(Ta1 a1);
	typedef SerializableFunctionPointer1 mytype;

	myfptype function;
public:
	SerializableFunctionPointer1(std::nullptr_t = nullptr) :function(nullptr), ISerializableFunctionPointer(nullptr)
	{}
	SerializableFunctionPointer1(myfptype f, const char* name) : ISerializableFunctionPointer(name), function(f)
	{

	}
	virtual Tret operator()(Ta1 a1) { return function(a1); }
	virtual bool operator !() { return function == nullptr; }
	virtual  operator bool() { return function != nullptr; }
	bool operator==(myfptype test) { return test == function; }
	bool operator!=(myfptype test) { return test != function; }
	bool operator==(const mytype& test) { return test.function == function; }
	bool operator!=(const mytype& test) { return test.function != function; }

	virtual void* get_fpointer() { return function; }
};
template <typename Tret, typename Ta1, typename Ta2> class SerializableFunctionPointer2 : public ISerializableFunctionPointer
{
	typedef Tret(*myfptype)(Ta1 a1, Ta2 a2) ;
	typedef SerializableFunctionPointer2 mytype;

	myfptype function;
public:
	SerializableFunctionPointer2(std::nullptr_t =nullptr):function(nullptr), ISerializableFunctionPointer(nullptr)
	{}
	SerializableFunctionPointer2(myfptype f, const char* name) : ISerializableFunctionPointer(name), function(f)
	{
	}
	virtual Tret operator()(Ta1 a1, Ta2 a2) { return function(a1, a2); }
	virtual bool operator !() { return function == nullptr; }
	virtual  operator bool() { return function != nullptr; }
	bool operator==(myfptype test) { return test == function; }
	bool operator!=(myfptype test) { return test != function; }
	bool operator==(const mytype&test) { return test.function == function; }
	bool operator!=(const mytype& test) { return test.function != function; }

	virtual void* get_fpointer() { return function; }
};

template <typename Tret, typename Ta1> inline SerializableFunctionPointer1<Tret, Ta1 > CreateSFP(Tret(*f)(Ta1 a1), const char* name)
{
	return SerializableFunctionPointer1<Tret, Ta1>(f, name);
}
template <typename Tret, typename Ta1, typename Ta2> inline SerializableFunctionPointer2<Tret, Ta1, Ta2 > CreateSFP(Tret(*f)(Ta1 a1, Ta2 a2), const char* name)
{
	return SerializableFunctionPointer2<Tret, Ta1, Ta2>(f, name);
}



template <typename Tret, typename Ta1, typename Ta2, typename Ta3> class SerializableFunctionPointer3 : public ISerializableFunctionPointer
{
	typedef Tret(*myfptype)(Ta1, Ta2, Ta3);
	typedef SerializableFunctionPointer3 mytype;

	myfptype function;

public:
	SerializableFunctionPointer3(std::nullptr_t = nullptr) :function(nullptr), ISerializableFunctionPointer(nullptr)
	{}
	SerializableFunctionPointer3(myfptype f, const char* name) : ISerializableFunctionPointer(name), function(f)
	{

	}
	virtual Tret operator()(Ta1 a1, Ta2 a2, Ta3 a3) { return function(a1, a2,a3); }
	virtual bool operator !() { return function == nullptr; }
	virtual  operator bool() { return function != nullptr; }
	bool operator==(myfptype test) { return test == function; }
	bool operator!=(myfptype test) { return test != function; }
	bool operator==(const mytype& test) { return test.function == function; }
	bool operator!=(const mytype& test) { return test.function != function; }

	virtual void* get_fpointer() { return function; }
};
template <typename Tret, typename Ta1, typename Ta2, typename Ta3> inline SerializableFunctionPointer3<Tret, Ta1, Ta2, Ta3> CreateSFP(Tret(*f)(Ta1 a1, Ta2 a2, Ta3 a3), const char* name)
{
	return SerializableFunctionPointer3<Tret, Ta1, Ta2, Ta3>(f, name);
}

template <typename Tret, typename Ta1, typename Ta2, typename Ta3, typename Ta4> class SerializableFunctionPointer4 : public ISerializableFunctionPointer
{
	typedef Tret(*myfptype)(Ta1, Ta2, Ta3, Ta4);
	typedef SerializableFunctionPointer4 mytype;

	myfptype function;

public:
	SerializableFunctionPointer4(std::nullptr_t = nullptr) :function(nullptr), ISerializableFunctionPointer(nullptr)
	{}
	SerializableFunctionPointer4(myfptype f, const char* name) : ISerializableFunctionPointer(name), function(f)
	{

	}
	virtual Tret operator()(Ta1 a1, Ta2 a2, Ta3 a3, Ta4 a4) { return function(a1, a2, a3, a4); }
	virtual bool operator !() { return function == nullptr; }
	virtual  operator bool() { return function != nullptr; }
	bool operator==(myfptype test) { return test == function; }
	bool operator!=(myfptype test) { return test != function; }
	bool operator==(const mytype& test) { return test.function == function; }
	bool operator!=(const mytype& test) { return test.function != function; }

	virtual void* get_fpointer() { return function; }
};


template <typename Tret, typename Ta1, typename Ta2, typename Ta3, typename Ta4> inline SerializableFunctionPointer4<Tret, Ta1, Ta2, Ta3, Ta4> CreateSFP(Tret(*f)(Ta1 a1, Ta2 a2, Ta3 a3, Ta4 a4), const char* name)
{
	return SerializableFunctionPointer4<Tret, Ta1, Ta2, Ta3, Ta4>(f, name);
}

template <typename Tret, typename Ta1, typename Ta2, typename Ta3, typename Ta4, typename Ta5> class SerializableFunctionPointer5 : public ISerializableFunctionPointer
{
	typedef Tret(*myfptype)(Ta1, Ta2, Ta3, Ta4, Ta5);
	typedef SerializableFunctionPointer5 mytype;

	myfptype function;

public:
	SerializableFunctionPointer5(std::nullptr_t = nullptr) :function(nullptr), ISerializableFunctionPointer(nullptr)
	{}
	SerializableFunctionPointer5(myfptype f, const char* name) : ISerializableFunctionPointer(name), function(f)
	{

	}
	virtual Tret operator()(Ta1 a1, Ta2 a2, Ta3 a3, Ta4 a4, Ta5 a5) { return function(a1, a2, a3, a4,a5); }
	virtual bool operator !() { return function == nullptr; }
	virtual  operator bool() { return function != nullptr; }
	bool operator==(myfptype test) { return test == function; }
	bool operator!=(myfptype test) { return test != function; }
	bool operator==(const mytype& test) { return test.function == function; }
	bool operator!=(const mytype& test) { return test.function != function; }

	virtual void* get_fpointer() { return function; }
};
template <typename Tret, typename Ta1, typename Ta2, typename Ta3, typename Ta4, typename Ta5> inline SerializableFunctionPointer5<Tret, Ta1, Ta2, Ta3, Ta4, Ta5> CreateSFP(Tret(*f)(Ta1 a1, Ta2 a2, Ta3 a3, Ta4 a4, Ta5 a5), const char* name)
{
	return SerializableFunctionPointer5<Tret, Ta1, Ta2, Ta3, Ta4, Ta5>(f, name);
}


#define SFPEnt(field,name) namespace SFP{ __declspec(selectany) decltype(edict_s::##field) name(::name,#name);}
#define SFPX(field,name) namespace SFP{ __declspec(selectany) decltype(field) name(::name,#name);}

///#define ExternSFPEnt(field,name) namespace SFP{ extern decltype(((edict_s*)0)->##field) name;}
#define AutoSFP(name) namespace SFP {__declspec(selectany) auto name = CreateSFP(::name,#name);}
//#define ExternAutoSFP(name) namespace SFP{extern decltype(CreateSFP(::name,#name)) name;}
