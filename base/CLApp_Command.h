/**********************************************************
* Copyright(C) 2022 CAN-Lab Inc.
* Garnet Kim <garnet@can-lab.co.kr> - 2022.10.04
* FileName : CLApp_Command.h
* Auther   : Garnet Kim
* Modify   : 2022-10-04
**********************************************************/
#ifndef _CLAPP_COMMAND_H_
#define _CLAPP_COMMAND_H_

#define MAX_CMD_PARAM	4
#define MAX_TEXT_SIZE	64 //128 //1024

class CLAppCommand 
{
public:
#pragma pack(push, 1)
	typedef union _tagCmdSub
	{
		char			text[MAX_TEXT_SIZE];
		int 			size;
	} sub_data_t;

	typedef struct _tagCmdMsg
	{
		long			type;
		int				id;
		int				param[MAX_CMD_PARAM];
		sub_data_t		sub;
	} command_t;
#pragma pack(pop)

public:
    CLAppCommand(int id);
	CLAppCommand(command_t& cmd);
	CLAppCommand(command_t* pCmd);
	CLAppCommand(CLAppCommand& cmd);
	CLAppCommand(const CLAppCommand& cmd);
	CLAppCommand(CLAppCommand* pCmd);
	CLAppCommand(int id, int p0, int p1, int p2, int p3);
    CLAppCommand();
    virtual ~CLAppCommand() {}

private:
	command_t mCommand; 

public:
	command_t* getCommand() { return &mCommand; }

    int getId() const { return mCommand.id; }
    void setId(int id) { mCommand.id = id; }
    int getMaxDataCount() const { return MAX_CMD_PARAM; }
    int getData(int num) const;
    sub_data_t* getSub() { return &mCommand.sub; }
    void setData(int data, int num);
	void setData(int p0, int p1, int p2, int p3);
	void setAll(int id, int p0, int p1, int p2, int p3);

    CLAppCommand& operator=(CLAppCommand& msg);
	CLAppCommand& operator=(const CLAppCommand& cmd);
};

#endif  // _CLAPP_COMMAND_H_
