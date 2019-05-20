#include"s_mainfunc.h"

SOCKET socket_init(SOCKADDR_IN addrSrv)
{
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	int connectResult = connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));//与服务器进行连接
	if (connectResult == -1)
	{
		cout << "Connection Fucked Up" << endl;
		return INVALID_SOCKET;
	}
	else if (connectResult == 0)
		cout << "Connect Successfully" << endl;
	return sockClient;
}

void state_socket(SOCKET sockClient, vector<string>& wordList, vector<player>& playerList, vector<tester>& testerList)
{
	char recvBuf[MaxSize];//所有通信的接收缓冲区
	int accurateStateBytes = recv(sockClient, recvBuf, MaxSize, 0);//服务器从客户端接受状态码，判断进行哪一个操作
	if (accurateStateBytes != 2)
	{
		cout << "Recv State Err" << endl;
		closesocket(sockClient);
		return;
	}
	stateCode state = (stateCode)(recvBuf[0] - '0');
	switch (state)
	{
	case WORD_SYNC://状态码为1，将服务器的单词列表发给客户端
	{
		int accurateByte = 0;
		for (unsigned int i = 0; i < wordList.size(); i++)
		{
			accurateByte = send(sockClient, wordList[i].data(), wordList[i].length(), 0);
			if (accurateByte != wordList[i].length())
			{
				cout << "WORD_SYNC 01" << endl;
				break;
			}
		}
		break;
	}
	case USER_REGIST://状态码为2，接收一个字符串："账户类型,用户名,密码"；
	{
		int accurateByte = recv(sockClient, recvBuf, MaxSize, 0);
		if (accurateByte <= 0)
		{
			cout << "USER_REGIST ERR 02_1" << endl;
			closesocket(sockClient);
			return;
		}
		string tmpString = recvBuf;
		vector<string>tmpUser;
		string_split(tmpString, tmpUser, ",");//tmpUser应该有3个元素，账户类型，用户名，密码
		int reg_flag = state_register(tmpUser, playerList, testerList);
		if (reg_flag == 0 || reg_flag == 1 || reg_flag == 2)
		{
			tmpString = std::to_string(reg_flag).data();
			accurateByte = send(sockClient, tmpString.data(), tmpString.length(), 0);
			if (accurateByte <= 0)
			{
				cout << "USER_REGIST ERR 02_2" << endl;
				closesocket(sockClient);
				return;
			}
		}
		break;
	}
	case USER_LOGIN:
	{
		int accurateByte = recv(sockClient, recvBuf, MaxSize, 0);//接收账户类型，用户名，密码
		if (accurateByte <= 0)
		{
			cout << "USER_LOGIN ERR 03_1" << endl;
			closesocket(sockClient);
			return;
		}
		string tmpString = recvBuf;
		vector<string>tmpUser;
		string_split(tmpString, tmpUser, ",");
		int login_flag = state_login(tmpUser, playerList, testerList);
		tmpString = std::to_string(login_flag).data();
		accurateByte = send(sockClient, tmpString.data(), tmpString.length(), 0);
		if (accurateByte != 2)
		{
			cout << "USER_LOGIN ERR 03_3" << endl;
		}
		break;
	}
	case PLAYER_RESULT:
	{
		int accurateByte = recv(sockClient, recvBuf, MaxSize, 0);//接收用户名，本轮游戏成绩
		if (accurateByte <= 0)
		{
			cout << "PLAYER_RESULT ERR 04_1" << endl;
			closesocket(sockClient);
			return;
		}
		string tmpString = recvBuf;
		vector<string>tmpUser;//tmpUser应该只有2个元素：用户名和本轮游戏轮数
		string_split(tmpString, tmpUser, ",");
		int flush_flag = state_player_result(tmpUser, playerList);
		if (flush_flag != 1)cout << "PLAYER_RESULT ERR 04_2" << endl;
		break;
	}
	case TESTER_RESULT:
	{
		int accurateByte = recv(sockClient, recvBuf, MaxSize, 0);//接收用户名
		if (accurateByte <= 0)
		{
			cout << "TESTER_RESULT ERR 06_1" << endl;
			break;
		}
		string name = recvBuf;
		vector<string> wordAddList;
		string tmpString;
		accurateByte = recv(sockClient, recvBuf, MaxSize, 0);//接收新单词
		while (accurateByte > 0)
		{
			tmpString = recvBuf;
			wordAddList.push_back(tmpString);
			accurateByte = recv(sockClient, recvBuf, MaxSize, 0);//接收新单词
		}
		int tester_flag = state_tester_result(name, wordAddList, wordList, testerList);
		if (tester_flag != 1)cout << "TESTER_RESULT ERR 06_2" << endl;
		break;
	}
	case USER_QUERY:
	{
		int accurateByte = recv(sockClient, recvBuf, MaxSize, 0);//接收用户名
		if (accurateByte <= 0)
		{
			cout << "USER_QUERY ERR 07_1" << endl;
			break;
		}
		string tmpString = recvBuf;
		vector<string> tmpInfo;
		string_split(tmpString, tmpInfo, ",");
		if (tmpInfo.size() != 2)
		{
			cout << "USER_QUERY ERR 07_2" << endl;
			break;
		}
		state_query(tmpInfo, playerList, testerList);//state_query把查询到的信息放到tmpInfo中
		accurateByte = send(sockClient, tmpInfo[0].data(), tmpInfo[0].length(), 0);
		for (unsigned int i = 1; i < tmpInfo.size(); i++)
		{
			accurateByte = send(sockClient, tmpInfo[i].data(), tmpInfo[i].length(), 0);
			if (accurateByte <= 0)
			{
				cout << "USER_QUERY ERR 07_3" << endl;
				break;
			}
		}
		break;
	}
	default:
	{
		cout << "state code err 00" << endl;
		break;
	}
	closesocket(sockClient);
	return;
	}
}