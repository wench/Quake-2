#include "APESTRUCTS.h"

namespace APE
{
	bool Command::Read(READER& reader, Command*& cmd)
	{
		ComCode code = (ComCode)reader.readu8();
		switch (code)
		{
		case CC_TITLE:
			cmd = new TitleCommand(reader);
			break;
		case CC_BODY:
			cmd = new BodyCommand(reader);
			break;
		case CC_CAM:
			cmd = new CamCommand(reader);
			break;
		case CC_TALK:
			cmd = new TalkCommand(reader);
			break;
		case CC_STYLE:
			cmd = new StyleCommand(reader);
			break;
		case CC_FONT:
			cmd = new FontCommand(reader);
				break;
		case CC_POSDIMS:
			cmd = new PosDimsCommand(reader);
			break;		
		case CC_IMAGE:
				cmd = new ImageCommand(reader);
				break;

		default:
		case CC_END:
			return false;
		}
		return true;
	}
	TitleCommand::TitleCommand(READER& reader)
	{
		while (reader.readu64() == 1)
		{
			//read an expression
			ExpressionValue* ev = new ExpressionValue(reader);
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
			ExpressionValue* ev = new ExpressionValue(reader);
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

			lvalue = new ExpressionValue(reader);
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
		while (Command::Read(reader, cmd))
		{
			if (cmd)commands.push_back(cmd);
			cmd = nullptr;
		}
		if (cmd)commands.push_back(cmd);
		cmd = nullptr;
	}

	Switch::Switch(READER& reader, int id) :seqid(id)
	{
		Command* cmd = nullptr;
		while (Command::Read(reader, cmd))
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
			auto done = reader.readu8();
			auto valueFlag = reader.readu8();
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
				xpos = new ExpressionValue(reader);
				if (reader.readi64() == 1);// I don't know what i should be doing here
			}
			else xpos = nullptr;
			if (reader.readi64() == 1)
			{
				ypos = new ExpressionValue(reader);
				if (reader.readi64() == 1);// I don't know what i should be doing here

			}
			else ypos = nullptr;
				if (reader.readi64() == 1)
				{
					width = new ExpressionValue(reader);
					if (reader.readi64() == 1);// I don't know what i should be doing here
				}
				else width = nullptr;
				if (reader.readi64() == 1)
				{
					height = new ExpressionValue(reader);
					if (reader.readi64() == 1);// I don't know what i should be doing here
				}
				else height = nullptr;
	}
		ImageCommand::ImageCommand(READER& reader)
		{

}
	}