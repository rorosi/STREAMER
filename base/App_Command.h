#ifndef _APP_COMMAND_H_
#define _APP_COMMAND_H_

#define MAX_CMD_PARAM	4
#define MAX_TEXT_SIZE	64 //128 //1024

class AppCommand 
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
    AppCommand(int id);
	AppCommand(command_t& cmd);
	AppCommand(command_t* pCmd);
	AppCommand(AppCommand& cmd);
	AppCommand(const AppCommand& cmd);
	AppCommand(AppCommand* pCmd);
	AppCommand(int id, int p0, int p1, int p2, int p3);
    AppCommand();
    virtual ~AppCommand() {}

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

    AppCommand& operator=(AppCommand& msg);
	AppCommand& operator=(const AppCommand& cmd);
};

#endif  // _APP_COMMAND_H_
