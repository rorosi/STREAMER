#include <string.h>

#include <App_Command.h>

AppCommand::AppCommand()
{
	memset(&mCommand, 0, sizeof(command_t));
}

AppCommand::AppCommand(int id)
{
	memset(&mCommand, 0, sizeof(command_t));
	mCommand.id = id;
}

AppCommand::AppCommand(command_t& cmd)
{
	memcpy(&mCommand, &cmd, sizeof(command_t));
}

AppCommand::AppCommand(command_t* pCmd)
{
	memcpy(&mCommand, pCmd, sizeof(command_t));
}

AppCommand::AppCommand(AppCommand& cmd)
{
	memcpy(&mCommand, cmd.getCommand(), sizeof(command_t));
}

AppCommand::AppCommand(const AppCommand& cmd)
{
	AppCommand* pCmd = (AppCommand*)&cmd;
	memcpy(&mCommand, pCmd->getCommand(), sizeof(command_t));
}

AppCommand::AppCommand(AppCommand* pCmd)
{
	memcpy(&mCommand, pCmd->getCommand(), sizeof(command_t));
}

AppCommand::AppCommand(int id, int p0, int p1, int p2, int p3)
{
	memset(&mCommand, 0, sizeof(command_t));
	mCommand.id = id;
	mCommand.param[0] = p0;
	mCommand.param[1] = p1;
	mCommand.param[2] = p2;
	mCommand.param[3] = p3;
}

int AppCommand::getData(int num) const 
{
	if (MAX_CMD_PARAM <= num) 
		num = 0;

	return mCommand.param[num];
}

void AppCommand::setData(int data, int num) 
{
	if (MAX_CMD_PARAM <= num) 
		return;

	mCommand.param[num] = data;
}

void AppCommand::setData(int p0, int p1, int p2, int p3)
{
	mCommand.param[0] = p0;
	mCommand.param[1] = p1;
	mCommand.param[2] = p2;
	mCommand.param[3] = p3;
}

void AppCommand::setAll(int id, int p0, int p1, int p2, int p3)
{
	mCommand.id = id;
	mCommand.param[0] = p0;
	mCommand.param[1] = p1;
	mCommand.param[2] = p2;
	mCommand.param[3] = p3;
}

AppCommand& AppCommand::operator=(AppCommand& cmd)
{
	memcpy(&mCommand, cmd.getCommand(), sizeof(command_t));

	return *this;
}

AppCommand& AppCommand::operator=(const AppCommand& cmd)
{
	AppCommand* pCmd = (AppCommand*)&cmd;
	memcpy(&mCommand, pCmd->getCommand(), sizeof(command_t));

	return *this;
}
