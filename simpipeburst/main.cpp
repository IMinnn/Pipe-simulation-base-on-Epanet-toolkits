#include "epanet2.h"
#include "pipeburst.h"
#include "Simpipeleak.h"
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
	char* check_point = "out/ky4/check_point.csv";
	char *pipe_inf = "out/ky4/pipe_cluster.csv";

	long time_step = 600;
	long time_duration = 15552000;
	Simpipeleak pipe(Inputfile);
	pipe.set_timeparam(600, 15552000);//设置时间步长为10min；周期为180天
	//pipe.set_timeparam(3600, 864000);//
	pipe.initialize();

	int leak_number = 1100; //一次模拟中，爆管发生次数
	int sim_number = 100;  //模拟数量
	int leak_scale = 1;   //爆管程度

	clock_t im_begin, im_end;
	double im_runtime;
	im_begin = clock();
	string pressure_file= "out/ky4/leakdata/check_normal_" + to_string(time_duration) + "_" + to_string(time_step) + ".csv";
	pipe.load_normal_pressure_data(pressure_file);
	im_end = clock();
	im_runtime = (double)(im_end - im_begin) / 1000;
	cout << "runtime:" << im_runtime << "s" << endl;

	for (int i = 1; i <= sim_number; i++)
	{
		leak_scale = 50;
		//if (i == 1)leak_scale = 5;
		//else if (i == 2)leak_scale = 10;
		//else if (i == 3)leak_scale = 50;
		//else if (i == 4)leak_scale = 100;
		
		clock_t this_time;
		this_time = clock();
		string str_this_time = to_string(this_time);
		char* str_time = (char*)str_this_time.data();

		string str1 = "out/ky4/leakdata/leak_" + str_this_time + "(" + to_string(leak_scale) + ")";
		string str2 = str1 + "/random_leak_inf.csv";
		char* leak_inf_file = (char*)str2.data();

		string normal_pressure = "out/ky4/leakdata/check_normal_"+to_string(time_duration)+"_"+to_string(time_step)+".csv";
		string out_file = "out/ky4/leakdata/leak_" + str_this_time +"("+to_string(leak_scale)+")"+ ".csv";

		LPCSTR   path = str1.c_str();
		CreateDirectory(path, NULL); //如果文件夹FileManege不存在，则创建。

		pipe.random_leak_node(pipe_inf, leak_inf_file, leak_number);

		//if(i==1)
		//{
		//	cout << endl << i << "/" << sim_number << "..." << endl;
			//cout << endl << i << "/" << sim_number << "..." << endl;
			//clock_t sim_begin, sim_end;
			//double sim_runtime;
			//sim_begin = clock();
		//	pipe.get_burstdata(out_file, check_point, leak_inf_file, normal_pressure, true, leak_scale);
			//sim_end = clock();
			//sim_runtime = (double)(sim_end - sim_begin) / 1000;
			//cout << "runtime:" << sim_runtime << "s" << endl;
		//}
		//else 
		//{
			cout << endl << i << "/" << sim_number << "..." << endl;
			clock_t sim_begin, sim_end;
			double sim_runtime;
			sim_begin = clock();
			pipe.get_burstdata(out_file, check_point, leak_inf_file, normal_pressure, false, leak_scale);\
			sim_end = clock();
			sim_runtime = (double)(sim_end - sim_begin) / 1000;
			cout << "runtime:" << sim_runtime << "s" << endl;
		//}
	}
}

void gettimepattern(char* inputfile, char* cluster_file)
{
	char* Inputfile = inputfile;
	char* check_point = "out/ky4/check_point.csv";
	char *pipe_inf = "out/ky4/pipe_cluster.csv";
	char* leak_inf_file = "out/ky4/random_leak.csv";
	char* normal_pressure = "out/ky4/check_normal.csv";
	Simpipeleak pipe(Inputfile);
	pipe.set_timeparam(600, 15552000);//设置时间步长为10min；周期为150天
	//pipe.set_timeparam(3600, 86400);
	pipe.initialize();

	pipe.gettimepattern();
}

int main()
{
	char* Inputfile = "data/Kentucky/ky4.inp";
	char* Outcata = "out/ky4/";
	char* cluster_file = "out/ky4/mod_cluster.csv";

	//Inputfile = "data/Changshucity/cs11021.inp";
	//Outcata = "out/cs/";
	//test1(Inputfile, Outcata);
	//test2(Inputfile,cluster_file);
	//test3(Inputfile, cluster_file);
	test4(Inputfile, cluster_file);
	//gettimepattern(Inputfile, cluster_file);
	
}
