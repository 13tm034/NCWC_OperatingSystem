#include <iostream>
#include <cstdlib>

#include "SharedMemory.h"

using namespace std;
const string FMNAME = "testmap";	//共有ファイル名
const string FMNAME_f_num = "testmap_f_num";	//共有ファイル名
const string FMNAME_pos_rad = "testmap_pos_rad";	//共有ファイル名

void unko_main()
{
	//型とファイルマッピングオブジェクト名を指定してインスタンス生成
	SharedMemory<int> shMem(FMNAME);
	//先に起動したc#のプロセスが共有メモリの先頭に書き込んだ値を取得
	cout << "Process# says => " << shMem.getShMemData() <<endl;
	
	//標準入力からint型の整数値を取得
	string str;
	cout << "Input number => " ;
	getline(cin,str);

	//int型の整数値を共有メモリの2番目の領域に書き込む
	shMem.setShMemData(atoi(str.c_str()), 1);

	getline(cin, str);
}

void main()
{
	int amount;
	int myNum;

	cout << "Please enter \"exit\" when you finish"  << endl;


	//型とファイルマッピングオブジェクト名を指定してインスタンス生成
	SharedMemory<int> shMem(FMNAME);
	SharedMemory<int> shMem_f_num(FMNAME_f_num);
	SharedMemory<float> shMem_pos_rad(FMNAME_pos_rad);

	if (shMem.isCreated()) {
		myNum = shMem.getShMemData();
		shMem.setShMemData(++myNum);
	}
	else
	{
		myNum = 1;
		shMem.setShMemData(1);
	}

	cout << "My process Number => " << myNum << endl;

	while (true){
		amount = shMem.getShMemData();
		//起動中のプロセスの数
		cout << "\nThe amount of the current process => " << amount << endl;

		for (int i = 1; i <= amount; i++)
		{
			cout << "f_num;" << shMem_f_num.getShMemData(0) << endl;
			cout << "x;" << shMem_pos_rad.getShMemData(0) << endl;
			cout << "y;" << shMem_pos_rad.getShMemData(1) << endl;
			cout << "rad;" << shMem_pos_rad.getShMemData(2) << endl;
			if (i != myNum)	cout << "Process" << i << " says => " << shMem.getShMemData(i) << endl;
		}

		//標準入力からint型の整数値を取得
		string str;
		cout << "\nInput number => ";
		getline(cin, str);

		//int型の整数値を共有メモリの2番目の領域に書き込む
		shMem.setShMemData(atoi(str.c_str()), myNum);

		getline(cin, str);
		if (str == "exit") break;
		
	}

}