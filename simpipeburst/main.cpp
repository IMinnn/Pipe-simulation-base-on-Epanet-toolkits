#include "epanet2.h"
#include "pipeburst.h"
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <vector>
#include <windows.h>
#include <sstream>
#include <map>
#include <random>
#include <cmath>
using namespace std;

//vector<int> random_list_leak(vector<string> temp_cluster);
//bool ifrepetition(int num, vector<int> list);

void test1(char* inputfile, char* outcata) 
{
	//输入
	char* Inputfile;
	//Inputfile = "data/cs11021.inp";
	//Inputfile = "data/cs1102(part).inp";
	//Inputfile = "data/Net3.inp";
	//Inputfile = "data/Kentucky/ky4.inp"; //此模型无负压力值
	//Inputfile = "data/ky8.inp";
	Inputfile = inputfile;
	if (!exists_test0(Inputfile))
	{
		cout << "inp file no exist" << endl;
		return ;
	}

	//输出
	char* pressure_before_delay ="pressure_before_delay.csv";
	string str11 = pressure_before_delay;
	string str12 = outcata + str11;
	pressure_before_delay = &str12[0];

	char* pressure_after_delay = "pressure_after_delay.csv";
	string str21 = pressure_after_delay;
	string str22 = outcata + str21;
	pressure_after_delay = &str22[0];

	char* pressure_change = "pressure_change_matrix.csv";	
	string str31 = pressure_change;
	string str32 = outcata + str31;
	pressure_change = &str32[0];

	char* nodexy = "nodexy.csv";
	string str41 = nodexy;
	string str42 = outcata + str41;
	nodexy = &str42[0];

	char* nodeid = "nodeid.csv";
	string str51 = nodeid;
	string str52 = outcata + str51;
	nodeid = &str52[0];

	char* log = "log.txt";
	string str61 = log;
	string str62 = outcata + str61;
	log = &str62[0];

	clock_t begin, end;
	double runtime;

	pipeburst pipe(Inputfile);
	pipe.set_timeparam(3600);//设置时间步长
	pipe.initialize();

	
	begin = clock();
	pipe.sim_before_delay(); //未设置爆管，模拟
	end = clock();
	runtime = (double)(end - begin) / 1000;
	std::cout << "run time:" << runtime << "s" << endl;
	pipe.out_file(pressure_before_delay, pipe.before_pressure); //输出至CSV文件


	begin = clock();
	pipe.sim_after_delay(true, log); //设置爆管，模拟
	end = clock();
	runtime = (double)(end - begin) / 1000;
	std::cout << "run time:" << runtime << "s" << endl;
	pipe.out_file(pressure_after_delay, pipe.after_pressure);	//输出至CSV文件

	pipe.pressure_change_matrix(pipe.before_pressure, pipe.after_pressure); //计算压变矩阵
	pipe.out_file(pressure_change, pipe.change_pressure); //输出至CSV文件
	
	pipe.get_nodexy(); //读取文件，获取节点坐标
	

	pipe.outfile_nodexy(nodexy, nodeid);
}


void test3(char* inputfile, char* file)
{
	char* Inputfile = inputfile;
	pipeburst pipe(Inputfile);
	pipe.set_timeparam(3600);//设置时间步长
	pipe.initialize();

	clock_t begin, end;
	double runtime;
	begin = clock();
	pipe.sim_before_delay(); //未设置爆管，模拟
	end = clock();
	runtime = (double)(end - begin) / 1000;
	std::cout << "run time:" << runtime << "s" << endl;


	pipe.random_leak_sim(file);
}

/*
void test2(char* file) 
{
	ifstream infile(file, ios::in);
	string linestr;
	vector<vector<string>> strarray;
	while (getline(infile, linestr))
	{
		//cout << linestr << endl;
		stringstream ss(linestr);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//按逗号分隔
			linearray.push_back(str);
		strarray.push_back(linearray);
	}
	
	vector<string> node_id;
	vector<int> node_cluster;
	int len = strarray.size();
	for (int i = 1; i < len; i++)
	{
		int cluster = stoi(strarray[i][1]);
		if (cluster != -1)
		{
			node_id.push_back(strarray[i][0]);
			node_cluster.push_back(stoi(strarray[i][1]));
		}
	}
	map<int, vector<string>> cluster_result;

	int len_cluster = node_id.size();
	int temp = node_cluster[0];
	vector<string> temp_map;
	for (int i = 0; i < len_cluster; i++)
	{
		if (node_cluster[i] == temp)
		{
			//cluster_result[node_cluster[i]] = node_id[i];
			temp_map.push_back(node_id[i]);
		}
		else
		{
			cluster_result[node_cluster[i - 1]] = temp_map;
			
			temp_map.clear();
			temp_map.push_back(node_id[i]);

			temp = node_cluster[i];
		}
	}

	map<int, vector<string>>::iterator it;
	int cluster_number = cluster_result.size();
	vector<vector<int>>leak_list;
	for (it = cluster_result.begin(); it!= cluster_result.end(); it++)
	{
		int cluster_value = it->first;
		vector<string> temp_cluster = it->second;
		int temp_number = temp_cluster.size();
		vector<int>leak_list_temp = random_list_leak(temp_cluster);
		leak_list.push_back(leak_list_temp);
	}


	int j = 0;
	j++;
}

vector<int> random_list_leak(vector<string> temp_cluster)
{
	int number = temp_cluster.size();
	int leak_number = 0;
	if (number <= 5)
	{
		leak_number = number;
	}
	else
	{
		leak_number = 5 + (int)sqrt(number - 25);
		
	}

	srand((unsigned int)time(0));
	vector<int> leak_list;
	int tmp = 0;
	int temp;

	for (int i = 0; i < leak_number; i++) {
		do 
		{
			temp = rand() % number;
			if (ifrepetition(temp, leak_list))continue;
			else break;
		} while (1);
		leak_list.push_back(temp);
		cout << temp << ", ";
	}
	cout << endl;
	return leak_list;
}

bool ifrepetition(int num, vector<int> list)
{
	int size = list.size();
	if (size == 0)return false;
	for (int i = 0; i < size; i++)
	{
		if (num == list[i])return true;
	}
	return false;
}

*/
int main()
{
	char* Inputfile = "data/Kentucky/ky4.inp";
	char* Outcata = "out/ky4/";
	char* cluster_file = "out/ky4/mod_cluster.csv";

	//Inputfile = "data/Changshucity/cs11021.inp";
	//Outcata = "out/cs/";
	//test1(Inputfile, Outcata);
	//test2(cluster_file);
	test3(Inputfile, cluster_file);

	
}
