#pragma once
#include "READER.h"
#include <list>
#include <unordered_map>
namespace APE
{
	class Expression;

	struct Value
	{
		virtual ~Value() {}

	};	class Statement
	{

	};
	class StatementValue : public Statement
	{
		std::list<Expression*> conditions;
		std::list<Value*> values;
	};


	struct floatvalue :public 	Value
	{
		floatvalue(READER & reader)
		{
			value = reader.readf32();
		}
		float value;
	};

	struct int16value :public 	Value
	{
		int16value(READER & reader)
		{
			value = reader.readi16();
		}
		int16_t value;
	};
	struct stringvalue :public Value
	{
		static stringvalue* readcond(READER& reader, size_t bytes = 0)
		{
			if (bytes == 0) bytes = reader.readu32();
			if (bytes) return new stringvalue(reader, bytes);
			return nullptr;
		}
		stringvalue() : value(nullptr) {}
		stringvalue(READER& reader, size_t bytes = 0)
		{
			if (bytes == 0) bytes = reader.readu32();
			char* str = new char[bytes ];
			reader.read(bytes, str);
			//str[bytes] = 0;
			value = str;
		}
		const char* value;
	};
	struct identifier :public Value
	{
		static identifier* readcond(READER& reader, size_t bytes = 0)
		{
			if (bytes == 0) bytes = reader.readu32();
			if (bytes) return new identifier(reader, bytes);
			return nullptr;
		}

		identifier() : varname(nullptr) {}
		identifier(READER& reader, size_t bytes = 0)
		{
			if (bytes == 0) bytes = reader.readu32();
			char* str = new char[bytes];
			reader.read(bytes,  str);
			//str[bytes] = 0;
			varname = str;
		}
		const char* varname;
	};
	enum OpCode
	{
		OP_OR = 1,
		OP_AND = 2,
		OP_XOR = 3,
		OP_GT = 4,
		OP_LT = 5,
		OP_GE = 6,
		OP_LE = 7,
		OP_EQ = 8,
		OP_ADD = 9,
		OP_SUB = 10,
		OP_MUL = 11,
		OP_DIV = 12,
		OP_NE = 13,
	};

	class Expression
	{
		virtual ~Expression() {}
		OpCode opcode;
		uint8_t valueflags;
		Value* lvalue;
		Value* rvalue;
	public:
		Expression(READER& reader);
	};
	class ExpressionValue : public Value
	{
		Expression* expr;
	public:
		ExpressionValue(READER& reader) {
			expr = new Expression(reader);

			// Lvalue
		}
	};
	enum ComCode
	{
		CC_IF = 1,
		CC_SETVAR = 2,
		CC_SETSTRING = 3,
		cc_WHILE = 11,
		CC_STARTSWITCH = 49,
		CC_THINKSWITCH = 50,
		CC_FINISHSWITCH = 51,
		CC_STARTCONSLE = 65,
		CC_BODY = 66,
		CC_CHOICE = 67,
		CC_BACKGROUND = 68,
		CC_END = 69,
		CC_FONT = 70,
		CC_POSDIMS = 71,
		CC_SUUBWINDOW = 72,
		CC_IMAGE = 73,
		CC_FLAGS = 76,
		CC_CAM = 77,
		CC_FINISHCONSOLE = 78,
		CC_NEXTWINDOW = 79,
		CC_XYPRIRINT = 80,
		CC_TITLE = 84,
		CC_STYLE = 87,
		CC_TALK = 89,
	};
	struct Command
	{
		virtual ~Command(){}
		ComCode code;
		std::list<Value*> conditions;

		static	bool Read(READER& reader, Command*& cmd);
	};
	struct SEQID
	{
		int id;
		__declspec(property(get = getBank)) int bank;
		__declspec(property(get = getEntry)) int entry;
		int getBank() { return id / 10000; }
		int getentry() { return id % 10000; }
		SEQID(int compiled) :id(compiled) {}
	};

	struct FormattingInfo
	{
		FormattingInfo(READER& reader);
		std::list<Value*> values;
	};


	struct TitleCommand :public Command
	{
		stringvalue title;
		FormattingInfo* formatting;
		TitleCommand(READER& reader);
	};
	struct BodyCommand :public Command
	{
		stringvalue body;
		FormattingInfo* formatting;
		BodyCommand(READER& reader);

	};
	struct TalkCommand :public Command
	{
		identifier* anim1, * anim2;
		identifier* name1, * name2;
		bool stay1, stay2;
		TalkCommand(READER& reader);

	};
	struct StyleCommand :public Command
	{
		identifier* style;

		StyleCommand(READER& reader);

	};
	struct FontCommand :public Command
	{
		identifier* font;

		FontCommand(READER& reader);

	};
	struct ImageCommand :public Command
	{
		stringvalue* filename;
		Value* xpos;
		Value* ypos;
		Value* width;
		Value* height;
		ImageCommand(READER& reader);

	};
	struct PosDimsCommand:public Command
	{
		PosDimsCommand(READER& reader);
		Value* xpos;
		Value* ypos;
		Value* width;
		Value* height;
	};
	struct CamCommand :public Command
	{
		identifier* name;
		Value *from;
		Value *to;
		Value *owner;
		int16_t yaw;
		int16_t pitch;
		int16_t fov;
		int16_t far;
		int16_t near;
		int16_t fwd;
		int16_t speed;
		int16_t lift;
		int16_t lag;
		int16_t occlude;
		bool restore;
		bool zip;
			CamCommand(READER& reader);

	};

	struct Window
	{
		SEQID seqid;
		std::list<Command*> commands;
		Window(READER& reader, int id);



	};

	struct Switch
	{

		SEQID seqid;
		std::list<Command*> commands;
		Switch(READER& reader, int id);

	};
}

