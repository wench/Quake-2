#include "APESTRUCTS.h"
#include <exception>
namespace APE
{
	int CountBits(uint64_t value)
	{
		/* value: The long sized value to count.

		Purpose : Count the number of leading 0 bits in the value.

		This has a one bit mask which it shifts from the high end to the
		low end of the long until the whole long is covered or a 1 bit is
		found.
		*/
		int cnt = 0;
		auto mask = 0x8000000000000000ULL;
		while (mask && !(value & mask))
		{
			cnt += 1;
			mask = mask >> 1;
		}
		return cnt;
	}
	template<typename Type1> bool Oneof(Type1 a1, Type1 c1)
	{
		return (a1 == c1);
	}	template<typename Type1, typename... Moretypes> bool Oneof(Type1 a1, Type1 c1, Moretypes... morec)
	{
		if (a1 == c1) return true;
		return Oneof(a1, morec...);
	}


		static uint64_t shiftMaskIn = 0xfffffffffffffffcull;
		static uint64_t shiftMaskInNew = 0x0000000000000003ull;
		static std::vector<Command*> *ReadNested(READER& reader, uint64_t cc, uint64_t parentccStart, bool is_if, uint64_t& ccend)
		{
			auto cntStart = CountBits(cc);
			static uint64_t ccStart = cc;
			uint64_t lastCC;
			std::vector< Command*>* commandlist = new std::vector<Command*>();

			// print "NSV - START     ", self.Binary(cc), self.parent is not None and self.parent.__class__.__name__, "|", self.name, self.condition
			while (1) {
				lastCC = cc;
				ComCode code;
				cc = ccStart;
				Command* cmd=nullptr;
				bool ret = (int)Command::Read(reader, cmd, code, cc);
				if (cmd) {
					cmd->code = code;
					commandlist->push_back(cmd);
				}
				if (!ret)
				{
					throw new std::exception();
					//raise RuntimeError("Unexpected ret", ret, cc)
				}
				//if cc is None :
				{
					//raise RuntimeError("Expected condition code", self)
				}
				auto lastCC0 = lastCC;
				auto lastCC1 = lastCC << 2;
				// Staying at the same level ?
				if ((cc & shiftMaskIn) != lastCC1)
				{
					int cntBefore = CountBits(lastCC); // Zero bits before the read command.
					int cntAfter = CountBits(cc);     // Zero bits after the read command.

					if (cntBefore < cntAfter)
					{
						if (is_if && (parentccStart & shiftMaskIn) == (cc & shiftMaskIn))
						{
							//print "NO EXIT (ccStart)", (self.Binary(self.ccStart), self.cntStart)
							//print "NO EXIT (  ccEnd)", (self.Binary(cc), cntAfter)
							//print "PARENT           ", self.parent is not None and (self.Binary(self.parent.ccStart), self.CountBits(self.parent.ccStart))
							//print "NSV - EXIT - 4"
							break;
						}
						else if ((cc & shiftMaskInNew) == 2)
						{
							int exitCount = 0;
							int cntBeforeShifted = cntBefore;
							while (cntBeforeShifted < cntAfter)
							{
								// Get the next sequence and move the lastCC to account for it.
								if ((lastCC & shiftMaskInNew) == 2)
								{
									exitCount += 1;
								}
									lastCC = lastCC >> 2;
									cntBeforeShifted += 2;
								

								if (exitCount > 0)
								{
									exitCount -= 1;
									// print "NSV - EXIT - 1"
									break;
								}

								//self.Add(None)
							}
						}
						else
						{
							// Last statement in a nesting.
								// print "NSV - EXIT - 2"
							break;
						}
					}
					else if (cntBefore == cntAfter and Oneof(lastCC & shiftMaskInNew,1ULL, 2ULL) and (cc & shiftMaskInNew) == 3)
					{
						// This was the only clause following an else.Drop out to give the next clause to the container.
							// print "NSV - EXIT - 3"
						break;
					}
					else if (cntBefore == cntAfter and (lastCC & shiftMaskInNew) == 1 and (cc & shiftMaskInNew) == 2)
					{
						//self.Add(None)
					}
					else
					{
						//	raise RuntimeError("Unexpected movement", Binary(lastCC, 64), cntBefore, Binary(cc, 64), cntAfter, cmds)
								//elif cc & shiftMaskInNew == 1:
												//   print "NSV - EXIT - 4"
											//    break
						throw new std::exception();;
					}
				}
				else if ((cc & shiftMaskInNew) != 3)
				{
					//print "NSV lastCC1", self.Binary(lastCC1)
						//print "NSV lastCC ", self.Binary(lastCC)
						//print "NSV cc     ", self.Binary(cc)
						//raise RuntimeError("Unexpected flags", cc & shiftMaskInNew)
					throw new std::exception();;
				}
			}
			ccend = cc;
			return commandlist;
		}
	bool Command::Read(READER& reader, Command*& cmd, ComCode &code, uint64_t &condcode)
	{
		 code = (ComCode)reader.readu8();
		switch (code)
		{
		case COM_IF:
			cmd = new NestedCommand(reader, condcode, true); \
				break;
		case COM_WHILE:
			cmd = new NestedCommand(reader, condcode, false); \
				break;
		case COM_BACKGROUND:
			cmd = new BGColourCommand(reader);
			break;
		case COM_CHOICE:
			cmd = new ChoiceCommand(reader);
			break;
		case COM_SUBWINDOW:
			if (reader.readi64() != 0)
				return false;
		case COM_STARTSWITCH:
		case COM_THINKSWITCH:
		case COM_FINISHSWITCH:
			cmd = new SEQIdARGCommand(reader);
			break;
		case COM_GOTO:
		case COM_GOSUB:
		case COM_CONSOLE:
		case COM_ECHO:
		case COM_TARGET:
		case COM_PATHTARGET:
		case COM_EXTERN:
		case COM_PLAYAMBIENT:
		case COM_LOOPAMBIENT:
		case COM_STOPAMBIENT:
		case COM_PLAYSCENE:
		case COM_LOOPSCENE:
		case COM_STOPSCENE:
		case COM_CHAINSCRIPTS:
		case COM_CLOSEWINDOW:
		case COM_LOADAPE:
			cmd = new StringARGCommand(reader, condcode, false);
		break;
		case COM_NEXTWINDOW:
		case COM_FINISHCONSOLE:
		case COM_STARTCONSLE:
			cmd = new StringARGCommand(reader, condcode,true);
			break;
		case COM_TITLE:
			cmd = new TitleCommand(reader);
			break;
		case COM_BODY:
			cmd = new BodyCommand(reader);
			break;
		case COM_CAM:
			cmd = new CamCommand(reader);
			break;
		case COM_TALK:
			cmd = new TalkCommand(reader);
			break;
		case COM_STYLE:
			cmd = new StyleCommand(reader);
			break;
		case COM_FONT:
			cmd = new FontCommand(reader);
				break;
		case COM_POSDIMS:
			cmd = new PosDimsCommand(reader);
			break;		
		case COM_IMAGE:
				cmd = new ImageCommand(reader);
				break;
		case COM_FLAGS:
			cmd = new FlagCommand(reader);
			break;
		case COM_XYPRIRINT:
			cmd = new XYPrintCommand(reader,condcode);
			break;

		case COM_SETVAR:
			cmd = new SetCommand(reader,condcode);
			break;

		case COM_SETSTRING:
			cmd = new SetCommand(reader,condcode,true);
			break;

		case COM_END:
			return false;
		default:
			throw new std::exception();

		}
		if (cmd) cmd->code = code;
		return true;
	}
	TitleCommand::TitleCommand(READER& reader)
	{
		while (reader.readu64() == 1)
		{
			//read an expression
			Value* ev = (new ExpressionValue(reader))->foldAndDelete();
			conditions.push_back(ev);
		}

		title = stringvalue(reader);
		formatting = new FormattingInfo(reader);

	}
	BodyCommand::BodyCommand(READER& reader)
	{
		while (reader.readu64() == 1)
		{
			//read an expression
			Value* ev = (new ExpressionValue(reader))->foldAndDelete();
			conditions.push_back(ev);

		}
		body = stringvalue(reader);
		formatting = new FormattingInfo(reader);
	}
	Expression::Expression(READER& reader) :lvalue(nullptr), rvalue(nullptr)
	{
		opcode = (OpCode)reader.readu8();
		valueflags = reader.readu8();

		// Lvalue
		reader.readu64(); // skip 8 bytes
		switch (valueflags)
		{
		case 0x00:
		case 0x08:
		case 0x0a:
		{

			lvalue = (new ExpressionValue(reader))->foldAndDelete();
		}
		break;

		case 0x04:
		case 0x0c:
		case 0x0e:
		{
			lvalue = new floatvalue(reader);

		}
		break;
		case 0x30:
		case 0x32:
		case 0x33:
		{
			lvalue = new stringvalue(reader);
		}
		break;


		case 0x05:
		case 0x0d:
		case 0x0f:
		case 0x31:
		{
			lvalue = new identifier(reader);
		}
		break;

		}

		// Rvalue
		reader.readu64(); // skip 4 bytes
		switch (valueflags)
		{
		case 0x00:
		case 0x04:
		case 0x05:
		{
			rvalue = new ExpressionValue(reader);
		}
		break;

		case 0x08:
		case 0x0c:
		case 0x0d:
		{
			rvalue = new floatvalue(reader);
		}
		break;

		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		{
			rvalue = new stringvalue(reader);
		}
		break;
		case 0x0a:
		case 0x0e:
		case 0x0f:
		{
			rvalue = new identifier(reader);
		}
		break;

		}
	}

	Window::Window(READER& reader, int id) :seqid(id)
	{
		Command* cmd = nullptr;
		ComCode code;
		uint64_t concode;
		while (Command::Read(reader, cmd, code, concode))
		{
			if (cmd) commands.insert(std::unordered_multimap<ComCode, Command*>::value_type(code, cmd));
			cmd = nullptr;
		}
		if (cmd)commands.insert(std::unordered_multimap<ComCode, Command*>::value_type(code, cmd));
		cmd = nullptr;
	}

	Switch::Switch(READER& reader, int id) :seqid(id)
	{
		Command* cmd = nullptr;
		ComCode code;
		uint64_t concode;
		while (Command::Read(reader, cmd, code, concode))
		{
			if (cmd)commands.push_back(cmd);
			cmd = nullptr;
		}
		if (cmd)commands.push_back(cmd);
		cmd = nullptr;
	}

	FormattingInfo::FormattingInfo(READER& reader)
	{
		auto done = reader.readu8();
		auto valueFlag = reader.readu8();
		while (!done)
		{
			if (valueFlag == 0x05)
				values.push_back(new stringvalue(reader));
			else if (valueFlag == 0x04)
				values.push_back(new floatvalue(reader));
			else if (valueFlag == 0x10)
				values.push_back(new stringvalue(reader));
			else if (valueFlag == 0x11)
				values.push_back(new identifier(reader));
			else
				//throw new std::bad_exception();
				return;
			done = reader.readu8();
			valueFlag = reader.readu8();
		}
	}

	CamCommand::CamCommand(READER& reader)
	{
		name = new identifier(reader);
		from = stringvalue::readcond(reader);
		to = stringvalue::readcond(reader);
		owner = stringvalue::readcond(reader);
		yaw = reader.readi16();
		pitch = reader.readi16();
		fov = reader.readi16();
		far = reader.readi16();
		near = reader.readi16();
		fwd = reader.readi16();
		speed = reader.readi16();
		lift = reader.readi16();
		lag = reader.readi16();
		occlude = reader.readi16();
		restore = reader.readi16() != 0;
		zip = reader.readi16() != 0;
			
	}
	TalkCommand::TalkCommand(READER& reader)
	{
		anim1 = new identifier(reader);
		anim2 = identifier::readcond(reader);
		name1 = new identifier(reader);
		name2 = new identifier(reader);
		stay1 = reader.readi32() != 0;
		stay2 = reader.readi32() != 0;
	}
	StyleCommand::StyleCommand(READER& reader)
	{
		style = new identifier(reader);

	}

	FontCommand::FontCommand(READER& reader)
	{
		font = new identifier(reader);

	}
		PosDimsCommand::PosDimsCommand(READER& reader)
		{
			if (reader.readi64() == 1)
			{
				xpos = (new ExpressionValue(reader))->foldAndDelete();
				if (reader.readi64() == 1);// I don't know what i should be doing here
			}
			else xpos = nullptr;
			if (reader.readi64() == 1)
			{
				ypos = (new ExpressionValue(reader))->foldAndDelete();
				if (reader.readi64() == 1);// I don't know what i should be doing here

			}
			else ypos = nullptr;
				if (reader.readi64() == 1)
				{
					width = (new ExpressionValue(reader))->foldAndDelete();
					if (reader.readi64() == 1);// I don't know what i should be doing here
				}
				else width = nullptr;
				if (reader.readi64() == 1)
				{
					height = (new ExpressionValue(reader))->foldAndDelete();
					if (reader.readi64() == 1);// I don't know what i should be doing here
				}
				else height = nullptr;
	}
		ImageCommand::ImageCommand(READER& reader)
		{
			while (reader.readi64() == 1)
				conditions.push_back(new ExpressionValue(reader));

			filename = new stringvalue(reader);

			if (reader.readi64() == 1)
			{
				xpos = (new ExpressionValue(reader))->foldAndDelete();
				if (reader.readi64() == 1)// I don't know what i should be doing here
					throw new std::exception();
					
			}
			else xpos = nullptr;
			if (reader.readi64() == 1)
			{
				ypos = (new ExpressionValue(reader))->foldAndDelete();
				if (reader.readi64() == 1)// I don't know what i should be doing here
					throw new std::exception();
			}
			else ypos = nullptr;
			if (reader.readi64() == 1)
			{
				width = (new ExpressionValue(reader))->foldAndDelete();
				if (reader.readi64() == 1)// I don't know what i should be doing here
					throw new std::exception();
			}
			else width = nullptr;
			if (reader.readi64() == 1)
			{
				height = (new ExpressionValue(reader))->foldAndDelete();
				if (reader.readi64() == 1)// I don't know what i should be doing here
					throw new std::exception();
			}
			else height = nullptr;

			int flags = reader.readi32();
			stretch = (flags & 1) == 1;
			tile = (flags & 2) == 2;
			solid = (flags & 4) == 4;
			is_background = false;
			if (width == nullptr && height == nullptr)
			{
				floatvalue* x = dynamic_cast<floatvalue*>(xpos);
				floatvalue* y = dynamic_cast<floatvalue*>(ypos);
				if (x&& x->value == 0 && y&& y->value == 0)
					is_background = true;
			}
		}

		static uint64_t ReadExpressionSegment(READER& reader, Value*& expr1, Value*& expr2) 
		{
			auto exprFlag = reader.readi64();

			if (exprFlag == 1)
			{
				expr1 = (new ExpressionValue(reader))->foldAndDelete();
				exprFlag = reader.readi64();
				if (exprFlag == 1)
				{
					expr2 = (new ExpressionValue(reader))->foldAndDelete();
				}
				else if (exprFlag == 0)
				{
					expr2 = nullptr;
				}
			}
			else if (exprFlag == 0)
			{
				expr1 = nullptr;
				expr2 = nullptr;
			}
			else
			{
				throw new std::exception();
			}
			return reader.readu64();
		}
		XYPrintCommand::XYPrintCommand(READER& reader, uint64_t& concode): font(nullptr),formatting(nullptr)
		{
			Value* v1; Value *v2;
			concode = ReadExpressionSegment(reader, v1, v2);
			args.push_back(v1);
			args.push_back((new ExpressionValue(reader))->foldAndDelete());

			for (int i = 0; i < 6; i++)
			{
				if (reader.readi64() == 1)
				{
					args.push_back((new ExpressionValue(reader))->foldAndDelete());
				}
			}

			if (args.size() > 3)
			{
				for (int i = 0; i < 3; i++)
				{
					if (reader.readi64() == 1)
					{
						args.push_back((new ExpressionValue(reader))->foldAndDelete());
					}
				}
				font = new stringvalue(reader);
			}
			else if (reader.readi32() != 0)
			{
				throw new std::exception();
			}
			args.push_back(new stringvalue(reader));
			
			while (reader.readi64() == 1)
				conditions.push_back(new ExpressionValue(reader));
			formatting = new FormattingInfo(reader);
				
		}
		ChoiceCommand::ChoiceCommand(READER& reader)
		{
			while (reader.readi64() == 1)
				conditions.push_back(new ExpressionValue(reader));
			prompt = new stringvalue(reader);
			formatting = new FormattingInfo(reader);
			seqid.id = reader.readi32();

		}
		SetCommand::SetCommand(READER& reader, uint64_t& condcode, bool string)
		{
			var = new identifier(reader);
			formatting = new FormattingInfo(reader);
			Value* v1; Value* v2;
			condcode =ReadExpressionSegment(reader, v1, v2);
			value = v1;
			if (string) {
				const char* expression = var->varname;
				const char* equal = strchr(expression, '=');
				if (equal)
				{
					size_t  namesize = equal - expression;
					char* name = new char[namesize];
					strncpy(name, expression, namesize - 1);
					var->varname = name;
					value = new stringvalue(equal + 1);
					delete[] expression;
				}
			}
		}
		StringARGCommand::StringARGCommand(READER& reader, uint64_t& condcode, bool noformat)
		{
			arg = new stringvalue(reader);
			if (!noformat)
			{
				formatting = new FormattingInfo(reader);

				Value* v1; Value* v2;
				condcode = ReadExpressionSegment(reader, v1, v2);
			}
		}
		NestedCommand::NestedCommand(READER& reader, uint64_t& condcode, bool isif)
		{
			auto value = reader.readi32();
			if (value != 0)
			{
				throw new std::exception();

			}
			formatting = new FormattingInfo(reader);

			auto prevconcode = condcode;
			Value* v1; Value* v2;
			condcode = ReadExpressionSegment(reader, v1, v2);
			if ((condcode & 3) != 1)
			{
				throw new std::exception();
			}
			expr = v1;
			uint64_t ccend;
			commands = ReadNested(reader, condcode, prevconcode, isif, ccend);
			condcode = ccend;
		}
	}