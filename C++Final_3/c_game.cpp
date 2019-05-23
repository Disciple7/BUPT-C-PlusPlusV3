#include"c_mainfunc.h"
int state_game(vector<string>& wordList)//返回值为闯关数
{
	int rewards = 0;
	int round = 0;//第几轮
	int wordCount =5;
	int size = wordList.size();
	int round_time = 3000;
	int biasA = 0;
	int biasB = biasA + wordList.size() / 2;
	int bonus = 0;//时间奖励，每积累3次时间奖励可以换算成一关
	int health = 5;//错误5次则终止游戏

	string targetWord;
	string myWord="\0";
	time_t timer;

	cout << "Game Start ! Type \":q\" to Quit !" << endl;
	while(myWord!=":q"&&health>0)
	{
		if (wordCount == 0)
		{
			round++;//每轮round+1个单词。本轮单词完成后重置并进入下一轮。
			wordCount = round+1;
			if (biasA < (size /3)*2)
			{
				biasA++;
				biasB = biasA + (size - biasA) / 2;
			}
			else
			{
				biasB = size - 1;//最难时抽取整个词库后1/3的单词
			}
			round_time = round_time*size / (size + round);//round_time逐渐减少
			cout << "Next Round " << round << " ! " << wordCount << "word(s) left !" << endl;
		}
		targetWord = wordList[biasA+rand()%(biasB-biasA)];//限定抽取范围随等级变化
		cout << targetWord;
		Sleep(round_time);
		cout << "\r                   \r";//清除一行，并把光标移到开始处
		timer = time(NULL);
		cin >> myWord;
		if (targetWord == myWord)
		{
			wordCount--;
			timer = time(NULL) - timer;
			cout << "Correct !" << endl;
			if (timer < 3)
			{
				cout << "Fast Type! Bonus! " << endl;
				bonus++;
			}
		}
		else if(myWord !=":q")
		{
			cout << "Wrong !" << endl;
			health--;
		}
	}
	cout << "Your Best Round：" << round << endl;
	return round+bonus/3;
}
void flush_player(player& nowPlayer,int bestRound, SOCKADDR_IN addrSrv)
{
	int flush_flag = -1;
	SOCKET sockClient = socket_init(addrSrv);
	string tmpString = nowPlayer.get_name() + "," + std::to_string(bestRound);
	flush_flag = state_socket(PLAYER_RESULT, tmpString, sockClient);
	closesocket(sockClient);
	for (int i = 0; flush_flag != 0 && i < 2; i++)
	{
		cout << "Reconnecting......" << endl;
		sockClient = socket_init(addrSrv);
		flush_flag = state_socket(PLAYER_RESULT, tmpString, sockClient);
		closesocket(sockClient);
	}
	if (flush_flag == 0)cout << "Result Upload Success" << endl;
	else cout << "Result Upload Failure" << endl;
	//更新此人的经验、等级、最佳轮数。如果发送一次失败就再发送一次。发送三次失败时则返回失败。
	return;
}
