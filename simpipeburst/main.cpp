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

	pipe.pressure_change(pipe.before_pressure, pipe.after_pressure); //计算压变矩阵
	pipe.out_file(pressure_change, pipe.change_pressure); //输出至CSV文件
	
	pipe.get_nodexy(); //读取文件，获取节点坐标
	

	pipe.outfile_nodexy(nodexy, nodeid);
}


void test3(char* inputfile, char* file)
{
	char* Inputfile = inputfile;
	char *random_file = "out/ky8/random_sim_result.csv";
	char *testdata_file = "out/ky8/testdata.csv";
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

	//pipe.random_leak_sim(file,random_file);

	pipe.all_leak_sim(file, testdata_file);
}


void test2(char* inputfile, char* cluster_file)
{
	char* Inputfile = inputfile;
	char *random_file = "out/ky4/random_sim_result.csv";
	char *testdata_file = "out/ky4/change_matrix.csv";
	pipeburst pipe(Inputfile);
	pipe.set_timeparam(3600);//设置时间步长
	pipe.initialize();

	clock_t begin, end;
	double runtime;
	begin = clock();
	
	pipe.get_change__matrix(cluster_file, testdata_file);
	
	end = clock();
	runtime = (double)(end - begin) / 1000;
	std::cout << "run time:" << runtime << "s" << endl;
	
	//pipe.get_change__matrix(cluster_file, testdata_file);
	
}

void test4(char* inputfile, char* cluster_file)
{
	char* Inputfile = inputfile;
	char *check_point = "out/ky4/check_point.csv";
	char *testdata_file = "out/ky4/change_matrix.csv";
	pipeburst pipe(Inputfile);
	pipe.set_timeparam(600);//设置时间步长
	pipe.initialize();

	pipe.get_test(check_point);
}

int main()
{
	char* Inputfile = "data/Kentucky/ky8.inp";
	char* Outcata = "out/ky8/";
	char* cluster_file = "out/ky8/mod_cluster.csv";

	//Inputfile = "data/Changshucity/cs11021.inp";
	//Outcata = "out/cs/";
	//test1(Inputfile, Outcata);
	//test2(Inputfile,cluster_file);
	//test3(Inputfile, cluster_file);
	test4(Inputfile, cluster_file);

	
}
