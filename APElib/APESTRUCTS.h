#pragma once
#include "READER.h"
#include <list>
#include <unordered_map>
namespace APE
{
	class Expression;
	
	class VariableManager {
	public:
		virtual float GetFloat(const char*varname);
		virtual const char* GetString(const char*varname);

	};

	struct Value
	{
		virtual ~Value() {}
		virtual float GetFloat(VariableManager* vm)
		{
			return (float) atof(GetString(vm, 0, 0));
		}
		
		virtual const char* GetString(VariableManager* vm, char* temporary, int tempsize)
		{
			if(temporary&& tempsize>=16)
				sprintf_s(temporary, tempsize, "%f", GetFloat(vm));
			return temporary;
		}

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
		floatvalue(float v) :value(v) {}
		floatvalue(READER& reader)
		{
			value = reader.readf32();
		}
		float value;
		virtual float GetFloat(VariableManager* vm) { return value; }


	};
	struct stringvalue :public Value
	{
		static stringvalue* readcond(READER& reader, size_t bytes = 0)
		{
			if (bytes == 0) bytes = reader.readu32();
			if (bytes) return new stringvalue(reader, bytes);
			return nullptr;
		}
		virtual ~stringvalue()
		{
			delete[] value;
		}
		stringvalue(const char* v) :value() {
			size_t bytes = strlen(v)+1;
			char* str = new char[bytes];
			strcpy_s(str, bytes, v);
			//str[bytes] = 0;
			value = str;
		}

		stringvalue() : value(nullptr) {}
		stringvalue(READER& reader, size_t bytes = 0)
		{
			if (bytes == 0) bytes = reader.readu32();
			char* str = new char[bytes];
			reader.read(bytes, str);
			//str[bytes] = 0;
			value = str;
		}
		const char* value;


		virtual const char* GetString(VariableManager* vm, char* temporary, size_t tempsize)
		{
			if (strlen(value) < tempsize)
			{
				strcpy(temporary, value);
				return temporary;
			}
			else return value;
			
		}
	};
	struct identifier :public Value
	{
		identifier(const char* v) :varname(v) {}


		virtual float GetFloat(VariableManager *vm)
		{
			if (vm) return vm->GetFloat(varname);
			else
			{
				return 0;
			}
		}
		virtual const char* GetString(VariableManager* vm, char* temporary, size_t tempsize)
		{
			if (vm) return vm->GetString(varname);
			else {
				temporary[0] = 0;

				return temporary;
			}
		}
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
			reader.read(bytes, str);
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
	public:
		OpCode opcode;
		uint8_t valueflags;
		Value* lvalue;
		Value* rvalue;
		Expression(READER& reader);
		virtual ~Expression()
		{
			delete lvalue;
			delete rvalue;
		}
		
	};
	class ExpressionValue : public Value
	{
		Expression* expr;
	public:
		ExpressionValue(READER& reader) {
			expr = new Expression(reader);
		}

		virtual ~ExpressionValue()
		{
			delete expr;
		}
		Value* foldAndDelete()
		{
			Value* folded = fold();
			if (folded != this) {
				delete this;
			}
			return folded;
		}

		Value* fold()
		{
			floatvalue* lvalue = dynamic_cast<floatvalue*>(expr->lvalue);
			floatvalue* rvalue = dynamic_cast<floatvalue*>(expr->rvalue);
			Value* folded = nullptr;
			if (lvalue && rvalue) {
				float res;
				float l = lvalue->value;
				bool bl = l != 0;
				float r = rvalue->value;
				bool br = r != 0;
				switch (expr->opcode)
				{
				case OP_OR:
					res = bl || br;
					break;
				case OP_AND:
					res = bl && br;
					break;
				case OP_XOR:
					res =bl || br && !(bl && br);
					break;
				case OP_GT:
					res = l > r;
					break;
				case OP_LT:
					res = l < r;
					break;
				case OP_GE:
					res = l >= r;
					break;
				case OP_LE:
					res = l <= r;
					break;
				case OP_EQ:
					res = l == r;
					break;
				case OP_ADD:
					res = l + r;
					break;
				case OP_SUB:
					res = l - r;
					break;
				case OP_MUL:
					res = l * r;
					break;
				case OP_DIV:
					res = l / r;
					break;
				case OP_NE:
					res = l != r;
					break;
				default:
					return this;
				}
				folded = new floatvalue(res);
			}
			if (folded)
			{

				return folded;
			}
			else if (lvalue && expr->opcode == OP_ADD)
			{
				folded = expr->rvalue;
				expr->rvalue = nullptr;
				return folded;
			}
			else
				return this;
		}
		virtual float GetFloat(VariableManager* vm)
		{
			 {
				float res;
				float l = expr->lvalue->GetFloat(vm);
				bool bl = l != 0;
				float r = expr->rvalue->GetFloat(vm);
				bool br = r != 0;
				switch (expr->opcode)
				{
				case OP_OR:
					res = bl || br;
					break;
				case OP_AND:
					res = bl && br;
					break;
				case OP_XOR:
					res = bl || br && !(bl && br);
					break;
				case OP_GT:
					res = l > r;
					break;
				case OP_LT:
					res = l < r;
					break;
				case OP_GE:
					res = l >= r;
					break;
				case OP_LE:
					res = l <= r;
					break;
				case OP_EQ:
					res = l == r;
					break;
				case OP_ADD:
					res = l + r;
					break;
				case OP_SUB:
					res = l - r;
					break;
				case OP_MUL:
					res = l * r;
					break;
				case OP_DIV:
					res = l / r;
					break;
				case OP_NE:
					res = l != r;
					break;
				default:
					return 0;
				}
				return res;
			}
		}
	};
	enum ComCode
	{
		COM_IF = 1,
		COM_SETVAR = 2,
		COM_SETSTRING = 3,
		COM_GOTO = 4,
		COM_GOSUB=5,
		COM_CONSOLE=6,
		COM_ECHO=7,
		COM_TARGET=8,
		COM_PATHTARGET=9,
		COM_EXTERN = 10,
		COM_PLAYAMBIENT = 12,
		COM_LOOPAMBIENT= 13,
		COM_STOPAMBIENT= 14,
		COM_PLAYSCENE=15,
		COM_LOOPSCENE=16,
			COM_STOPSCENE=17,
			COM_CHAINSCRIPTS= 18,
			COM_CLOSEWINDOW= 19,
		COM_LOADAPE=20,
		COM_WHILE = 11,
		COM_STARTSWITCH = 49,
		COM_THINKSWITCH = 50,
		COM_FINISHSWITCH = 51,
		COM_STARTCONSLE = 65,
		COM_BODY = 66,
		COM_CHOICE = 67,
		COM_BACKGROUND = 68,
		COM_END = 69,
		COM_FONT = 70,
		COM_POSDIMS = 71,
		COM_SUBWINDOW = 72,
		COM_IMAGE = 73,
		COM_FLAGS = 76,
		COM_CAM = 77,
		COM_FINISHCONSOLE = 78,
		COM_NEXTWINDOW = 79,
		COM_XYPRIRINT = 80,
		COM_TITLE = 84,
		COM_STYLE = 87,
		COM_TALK = 89,
	};
	struct Command
	{
		virtual ~Command() {}
		ComCode code;
		std::list<Value*> conditions;

		static	bool Read(READER& reader, Command*& cmd, ComCode &code, uint64_t& concode);
	};
	struct SEQID
	{
		__declspec(property(get = getId, put = setId)) int id;
		int bank;
		int entry;
		int getId()
		{
			return bank * 10000 + entry;
		}
		void setId(int val)
		{
			bank = val / 10000; 
			entry = val % 10000; 
		}
		SEQID(int val) { id = val; }
		SEQID() :bank(0), entry(0) {}
	};

	struct FormattingInfo
	{
		FormattingInfo(READER& reader);
		std::list<Value*> values;
	};


	struct FlagCommand : public Command
	{
		uint32_t flags;
		FlagCommand(READER& reader)
		{
			flags = reader.readu32();
		}
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
	struct XYPrintCommand :public Command
	{
		
		std::vector<Value*> args;
		Value* font;
		FormattingInfo* formatting;
		XYPrintCommand(READER& reader, uint64_t& concode);
	};
	struct ImageCommand :public Command
	{
		stringvalue* filename;
		Value* xpos;
		Value* ypos;
		Value* width;
		Value* height;
		bool stretch;
		bool tile;
		bool solid;
		bool is_background;

		ImageCommand(READER& reader);

	};
	struct PosDimsCommand :public Command
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
		Value* from;
		Value* to;
		Value* owner;
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
		std::unordered_multimap<ComCode,Command*> commands;
		Window(READER& reader, int id);
		void AddCommand(Command* cmd);


	};

	struct StringARGCommand :public Command
	{
		stringvalue* arg;
		FormattingInfo* formatting;
		StringARGCommand(READER& reader, uint64_t& condcode, bool noformat);
		virtual ~StringARGCommand()
		{
			delete arg;
		}

	};

	struct ChoiceCommand :public Command
	{
		stringvalue* prompt;
		std::vector<Value*> args;
		SEQID seqid;
		FormattingInfo* formatting;
		ChoiceCommand(READER& reader);
	};

	struct SetCommand :public Command
	{
		identifier* var;
		FormattingInfo* formatting;
		Value* value;

		SetCommand(READER& reader, uint64_t& condcode, bool string=false);

	};
	struct SEQIdARGCommand :public Command
	{
		SEQID arg;
		SEQIdARGCommand(READER& reader)
		{
			arg.id = reader.readi32();
		}
	};
	struct BGColourCommand :public Command
	{
		uint8_t rgba[4];
		BGColourCommand(READER& reader)
		{
			reader.read(4, rgba);
				
		}

	};
	struct Switch
	{

		SEQID seqid;
		std::vector<Command*> commands;
		Switch(READER& reader, int id);
	};
	struct NestedCommand: public Command
	{
		Value* expr;
		FormattingInfo* formatting;
		std::vector<Command*>* commands;
		NestedCommand(READER& reader, uint64_t& condcode,bool isif);
	};
}

