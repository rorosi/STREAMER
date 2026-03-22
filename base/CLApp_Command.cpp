/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Command.cpp
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#include <string.h>

#include <CLApp_Command.h>

CLAppCommand::CLAppCommand()
{
	memset(&mCommand, 0, sizeof(command_t));
}

CLAppCommand::CLAppCommand(int id)
{
	memset(&mCommand, 0, sizeof(command_t));
	mCommand.id = id;
}

CLAppCommand::CLAppCommand(command_t& cmd)
{
	memcpy(&mCommand, &cmd, sizeof(command_t));
}

CLAppCommand::CLAppCommand(command_t* pCmd)
{
	memcpy(&mCommand, pCmd, sizeof(command_t));
}

CLAppCommand::CLAppCommand(CLAppCommand& cmd)
{
	memcpy(&mCommand, cmd.getCommand(), sizeof(command_t));
}

CLAppCommand::CLAppCommand(const CLAppCommand& cmd)
{
	CLAppCommand* pCmd = (CLAppCommand*)&cmd;
	memcpy(&mCommand, pCmd->getCommand(), sizeof(command_t));
}

CLAppCommand::CLAppCommand(CLAppCommand* pCmd)
{
	memcpy(&mCommand, pCmd->getCommand(), sizeof(command_t));
}

CLAppCommand::CLAppCommand(int id, int p0, int p1, int p2, int p3)
{
	memset(&mCommand, 0, sizeof(command_t));
	mCommand.id = id;
	mCommand.param[0] = p0;
	mCommand.param[1] = p1;
	mCommand.param[2] = p2;
	mCommand.param[3] = p3;
}

int CLAppCommand::getData(int num) const 
{
	if (MAX_CMD_PARAM <= num) 
		num = 0;

	return mCommand.param[num];
}

void CLAppCommand::setData(int data, int num) 
{
	if (MAX_CMD_PARAM <= num) 
		return;

	mCommand.param[num] = data;
}

void CLAppCommand::setData(int p0, int p1, int p2, int p3)
{
	mCommand.param[0] = p0;
	mCommand.param[1] = p1;
	mCommand.param[2] = p2;
	mCommand.param[3] = p3;
}

void CLAppCommand::setAll(int id, int p0, int p1, int p2, int p3)
{
	mCommand.id = id;
	mCommand.param[0] = p0;
	mCommand.param[1] = p1;
	mCommand.param[2] = p2;
	mCommand.param[3] = p3;
}

CLAppCommand& CLAppCommand::operator=(CLAppCommand& cmd)
{
	memcpy(&mCommand, cmd.getCommand(), sizeof(command_t));

	return *this;
}

CLAppCommand& CLAppCommand::operator=(const CLAppCommand& cmd)
{
	CLAppCommand* pCmd = (CLAppCommand*)&cmd;
	memcpy(&mCommand, pCmd->getCommand(), sizeof(command_t));

	return *this;
}
