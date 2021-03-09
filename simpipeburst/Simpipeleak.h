#pragma once

#include "epanet2.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <string>
#include <windows.h>
#include <sstream>
#include <map>
#include <random>
#include <cmath>
#include<numeric>

#include "pipeburst.h"
using namespace std;


#define sFlow 0.5       //
#define scale_pressure 100      //压力变化扩大系数



class Simpipeleak
{
public:

	int node_num;              //管网总结点数
	long time_step;           //水力模拟时间步长，默认3600s
	long time_duration;       //模拟总历时，默认24h
	vector<string> node_id;    //所有结点ID
	vector<int> node_type;    //所有结点类型
	vector<long> timestep;     //延时模拟时间戳
	vector<vector<float>> before_pressure; //正常状态下水力模拟所有结点压力值,位置+1=index
	vector<vector<float>> before_pressure_ac; //正常状态下水力模拟所有结点压力值,位置+1=index
	vector<vector<float>> after_pressure;  //爆管状态下水力模拟所有结点压力值
	vector<vector<float>> change_pressure; //爆管前后压力变化矩阵（压变矩阵）


	Simpipeleak(char* inputFile);
	~Simpipeleak();
	//设置时间步长，默认3600s，周期为24h
	void set_timeparam(long step, long duration);
	//获取管网相关信息
	void initialize();
//获取独立结点各状态下压力信息，用于结点聚类
	//正常压力模拟，并获取爆管结点处压力值，记录在before_pressure数组中
	void sim_before_delay();
	// 逐个对所有结点处进行爆管模拟，并获取爆管结点处压力值，记录在before_pressure数组中
	void sim_after_delay(bool if_log, char* log_file);    
	//将before_pressure和before_pressure数组中对应节点相同时刻压力值作差值，计算压力变化，记录在change_pressure数组中
	void pressure_change(vector<vector<float>> be_pressure, vector<vector<float>> af_pressure); 
	//输出所有普通结点各时刻模拟的压力情况
	void out_file(char * file, vector<vector<float>> pressure);//将压力值数据写入CSV文件中



//设置爆管模拟，获取相关数据
	vector<vector<float>> ndoe_normal_sim(vector<string> node_list_);
	void get_burstdata(string out_file, char * pipefile, char * nodefile, string outfile, bool run_normal, int leak_scale = 1);
	void out_check_normal(string file_path, vector<vector<float>> pressure, vector<string> node_list_, vector<int> node_cluster_);
	void leak_pipe_simulate(vector<vector<string>>check_point_list, vector<vector<string>>leak_inf, vector<vector<float>>be_pressure);
	void leak_pipe_simulate_mutl(string out_file, vector<vector<string>>check_point_list, vector<vector<string>>leak_inf, vector<vector<float>>be_pressure, int leak_scale = 1);

	void out_check_leak(string file_path, vector<vector<float>> pressure, vector<string> leak_inf, vector<vector<string>>check_point_list);
	
	void out_check_leak_for_day(string file_path, vector<vector<float>> pressure, vector<vector<string>>check_point_list);
	
	void random_leak_node(char* clusterfile, char* out_leak_inf, int number );

	void gettimepattern();

	void setnodetimepattern(int pattern_index);

	void load_normal_pressure_data(string pressure_file);

private:
	char *outputfile;          //二进制输出文件，可忽略
	char *inputfile;           //管网文件
	char *report;              //报告文件
};



Simpipeleak::Simpipeleak(char* file)
{
	if (!exists_test0(file))
	{
		std::cout << "inp file no exist and pipeburst class create failed!" << endl;
		return;
	}
	else
	{
		inputfile = file;
		report = "report.txt", outputfile = "";        //忽略
		time_step = 3600;
		time_duration = 86400;
		std::cout << "pipeburst class create succeeded!" << endl;
	}
}

Simpipeleak::~Simpipeleak()
{
	std::cout << "pipeburst class delete succeeded!" << endl;
}

//设置时间步长，默认3600s，周期为24h
void Simpipeleak::set_timeparam(long step, long duration = 86400)
{
	time_duration = duration;
	time_step = step;
	cout << "Set step:"<<step<<"s，duration:"<<duration/3600/24<<"days." << endl;
}



//获取管网相关数据至成员变量
void Simpipeleak::initialize()
{
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return;
	}
	char node_id_temp[16];                      //节点ID
	int node_type_temp;                         //节点类型
	ENopen(inputfile, report, outputfile);

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);
	ENgetcount(EN_NODECOUNT, &node_num);		 //获取节点数
	for (int i = 1; i <= node_num; i++)
	{
		ENgetnodeid(i, node_id_temp);           //获取结点ID
		ENgetnodetype(i, &node_type_temp);      //获取结点类型
		node_id.push_back(node_id_temp);
		node_type.push_back(node_type_temp);
	}
	for (int i = 0; i <= time_duration / time_step;i++)
	{
		timestep.push_back(time_step*i);
	}

	/*
	long t, tstep;
	long t_check = 0;
	ENopenH();
	ENinitH(0);
	do
	{
		ENrunH(&t);
		if (t != t_check)
		{
			ENnextH(&tstep);
			continue;
		}
		timestep.push_back(t);
		ENnextH(&tstep);
		t_check += time_step;
	} while (tstep > 0);
	ENcloseH();
	ENclose();
	std::cout << "Get information for pipe network succeeded!!" << endl;
	int kk = 0;
	kk++;*/
}

//正常压力模拟，记录在before_pressure数组中
void Simpipeleak::sim_before_delay()
{
	std::cout << "开始正常水力模拟..." << endl;
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return;
	}

	ENopen(inputfile, report, outputfile); //打开文件

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);

	for (int i = 1; i <= node_num; i++)
	{
		ENopenH();
		ENinitH(0);
		long t, tstep;
		long t_check = 0;
		vector<float> before_pressure_temp;
		do
		{
			ENrunH(&t);
			if (t != t_check)
			{
				ENnextH(&tstep);
				continue;
			}
			float pressure;
			ENgetnodevalue(i, EN_PRESSURE, &pressure);
			before_pressure_temp.push_back(pressure);
			ENnextH(&tstep);
			t_check += time_step;
		} while (tstep > 0);
		ENcloseH();
		before_pressure.push_back(before_pressure_temp);
		printf("%.2lf%%\r", i * 100.0 / node_num);//进度条
	}
	std::cout << endl;
	ENclose();
	std::cout << "sim_before_delay succeeded!" << endl;
}

//逐个对所有结点处进行爆管模拟，并获取爆管结点处压力值，记录在before_pressure数组中
void Simpipeleak::sim_after_delay(bool if_log = false, char* log_file = NULL)
{
	std::cout << "开始爆管水力模拟..." << endl;
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return;
	}
	ofstream file_log;
	if (if_log) file_log.open(log_file, ios::out);
	ENopen(inputfile, report, outputfile); //打开文件

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);

	float fEmitterCoeff;
	for (int i = 1; i <= node_num; i++)
	{
		ENopenH();
		ENinitH(0);
		long t = 0, tstep;
		long t_check = 0;
		int j = 0;
		vector<float> after_pressure_temp;
		do
		{
			if (before_pressure[i - 1][j] < 0)//判断压力值是否为负,若为负则压力值不变
			{
				fEmitterCoeff = 0;
				if (if_log) file_log << "node:" << node_id[i - 1] << " ,time:" << t << "时刻模拟压力为负值" << endl;
			}
			else fEmitterCoeff = (float)sFlow* pow(before_pressure[i - 1][j], 0.5);  //计算喷射系数
			ENsetnodevalue(i, EN_EMITTER, fEmitterCoeff);
			ENrunH(&t);
			if (t != t_check)
			{
				ENnextH(&tstep);
				continue;
			}
			float pressure;
			ENgetnodevalue(i, EN_PRESSURE, &pressure);
			ENsetnodevalue(i, EN_EMITTER, 0);
			after_pressure_temp.push_back(pressure);
			ENnextH(&tstep);
			t_check += time_step;
			j++;
		} while (tstep > 0);
		ENcloseH();
		after_pressure.push_back(after_pressure_temp);
		printf("%.2lf%%\r", i * 100.0 / node_num);//进度条
	}
	std::cout << endl;
	ENclose();
	if (if_log) file_log.close();
	std::cout << "sim_after_delay succeeded!" << endl;
}

//将before_pressure和before_pressure数组中对应节点相同时刻压力值作差值，计算压力变化，记录在change_pressure数组中
void Simpipeleak::pressure_change(vector<vector<float>> be_pressure, vector<vector<float>> af_pressure)
{
	cout << "开始计算压力变化数据..." << endl;
	int col1, col2; //列大小
	int row1, row2; //行大小



	vector<float>size_temp = be_pressure[0];
	col1 = size_temp.size();
	size_temp = af_pressure[0];
	col2 = size_temp.size();
	row1 = be_pressure.size();
	row2 = af_pressure.size();

	if (col1 != col2 || row1 != row2 || col1 == 0 || row1 == 0)
	{
		std::cout << "calculate pressure matrix failed!" << endl;
		return;
	}

	for (int i = 0; i < row1; i++)
	{
		vector<float> pressure_temp;
		if (af_pressure[i].size() != be_pressure[i].size() || af_pressure[i].size() != timestep.size())
		{
			std::cout << "node：" << node_id[i] << "'s data is false! skip it!" << endl;

			continue;
		}
		for (int j = 0; j < col1; j++)
		{
			float del;
			if (be_pressure[i][j] == 0)//初始压力值为0，根据漏损公式计算得到的压力值也为0
				del = af_pressure[i][j];
			del = (be_pressure[i][j] - af_pressure[i][j]) / fabs(be_pressure[i][j]);
			pressure_temp.push_back(del*scale_pressure);
		}
		change_pressure.push_back(pressure_temp);
		//normal_nodeid.push_back(node_id[i]);
	}

	cout << "计算压变矩阵完成..." << endl;
}

//输出所有普通结点各时刻模拟的压力情况
void Simpipeleak::out_file(char * file_path, vector<vector<float>> pressure)
{

	if (pressure.size() == 0)
	{
		std::cout << "failed as no pressure data!" << endl;
		return;
	}
	ofstream file;
	file.open(file_path, ios::out);

	int len = timestep.size();
	file << " ,";
	for (int i = 0; i < len - 1; i++)
	{
		file << timestep[i] << ",";
	}
	file << timestep[len - 1] << endl;

	for (int i = 0; i < node_num; i++)
	{

		if (node_type[i] == 0)
		{
			if (pressure[i].size() != len)
			{
				std::cout << "node：" << node_id[i] << "'s data is false! skip it!" << endl;
				continue;
			}
			file << node_id[i] << ",";
			for (int j = 0; j < len; j++)
			{
				file << pressure[i][j] << ",";
			}
			file << endl;
		}
	}

	file.close();

	std::cout << " write to file succeeded!" << endl;
}


//正常情况下模拟
vector<vector<float>> Simpipeleak::ndoe_normal_sim(vector<string> node_list_)
{

	cout << endl;
	std::cout << "Start normal hydraulic simulation..." << endl;


	ENopen(inputfile, report, outputfile); //打开文件

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);
	int size1 = node_list_.size();
	int size2 = timestep.size();
	int size3 = size1 * size2;
	vector<vector<float>> normal_pressure;
	ENopenH();
	ENinitH(0);
	long t, tstep;
	long t_check = 0;
	int j = 0;
	do
	{
		ENrunH(&t);
		if (t != t_check)
		{
			ENnextH(&tstep);
			continue;
		}
		vector<float> pressure_temp;
		for (int i = 0; i < node_list_.size(); i++)
		{
			int node_index;
			string node_id = node_list_[i];
			char* node_id_ = (char*)node_id.data();
			ENgetnodeindex(node_id_, &node_index);
			float pressure;
			ENgetnodevalue(node_index, EN_PRESSURE, &pressure);
			pressure_temp.push_back(pressure);

			printf("%.2lf%%\r", (j * size1 + i) * 100.0 / size3);//进度条
		}
		normal_pressure.push_back(pressure_temp);
		ENnextH(&tstep);
		t_check += time_step;
		j++;
	} while (tstep > 0);
	ENcloseH();
	
	
	
	std::cout << endl;
	ENclose();
	std::cout << "Normal hydraulic simulation is successful!" << endl;
	return normal_pressure;
}


//获取监测点以及爆管设置信息，并进行正常以及爆管模拟，获取监测点压力数据
void Simpipeleak::get_burstdata(string out_file, char * nodefile, char * pipe_leak_inf_file, string out_noramal_file, bool run_normal, int leak_scale)
{
	//读取检测结点信息CSV文件，此处监测点默认为所有普通结点
	//文件格式要求：第一列为结点名称，第二列为结点类别，带表头
	ifstream infile_node(nodefile, ios::in);
	string linestr_node;
	vector<vector<string>> node_infarray;   //存储csv数据，二维数组
	while (getline(infile_node, linestr_node))
	{
		stringstream ss(linestr_node);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//按逗号分隔
			linearray.push_back(str);
		node_infarray.push_back(linearray);
	}

	node_infarray.erase(node_infarray.begin()); //删除表头数据元素

	vector<string> node_list;
	vector<int> node_cluster;
	//map<string, int> map_node_cluster;

	for (int i = 0; i < node_infarray.size(); i++)
	{
		node_list.push_back(node_infarray[i][0]);
		node_cluster.push_back(stoi(node_infarray[i][1]));
		//map_node_cluster[node_infarray[i][0]] = stoi(node_infarray[i][1]);
	}


	//读取爆管管道信息CSV文件
	//文件要求第一列为管道ID,第二列为管道类别。第三列为爆管时间，到表头
	ifstream infile_leakpipe(pipe_leak_inf_file, ios::in);
	string linestr_leakpipe;
	vector<vector<string>> leak_pipe_infarray;   //存储csv数据，二维数组
	while (getline(infile_leakpipe, linestr_leakpipe))
	{
		stringstream ss(linestr_leakpipe);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//按逗号分隔
			linearray.push_back(str);
		leak_pipe_infarray.push_back(linearray);
	}

	leak_pipe_infarray.erase(leak_pipe_infarray.begin());//删除文件表头元素

	//获取正常情况下，各监测点压力值
	//vector<vector<float>> normal_pressure;

	clock_t begin, end;
	double runtime;
	begin = clock();
	if (run_normal == true)
	{
		before_pressure = ndoe_normal_sim(node_list);
	}
	end = clock();
	runtime = (double)(end - begin) / 1000;
	std::cout << "run time:" << runtime << "s" << endl;


	//输出正常情况下监测点压力数据
	//out_check_normal(out_noramal_file, before_pressure, node_list, node_cluster); //输出至CSV文件
	//out_check_leak_for_day(out_noramal_file, before_pressure, node_infarray);
	//leak_pipe_simulate(node_infarray, leak_pipe_infarray, normal_pressure);
	leak_pipe_simulate_mutl(out_file, node_infarray, leak_pipe_infarray, before_pressure, leak_scale);
}



//将正常模拟后得到的监测点压力数据输出至文件，在get_burstdata（...）函数中调用
void Simpipeleak::out_check_normal(string file_path, vector<vector<float>> pressure, vector<string> node_list_, vector<int> node_cluster_)
{
	cout << endl;
	ofstream file;
	file.open(file_path, ios::out);

	int len = timestep.size();
	//file << "Cluster,";
	file << "ID,";
	for (int i = 0; i < len - 1; i++)
	{
		file << timestep[i] << ",";
	}
	file << timestep[len - 1] << endl;

	for (int i = 0; i < node_list_.size(); i++)
	{
		file << node_list_[i] << ",";
		for (int j = 0; j < len; j++)
		{
			file << pressure[j][i] << ",";
		}
		file << endl;
	}

	file.close();

	std::cout << "Normal hydraulic simulation data output successfully!" << endl;
}


//对管道进行爆管模拟，一次模拟只设置一次爆管事件
void Simpipeleak::leak_pipe_simulate(vector<vector<string>> check_point_list, vector<vector<string>> leak_inf, vector<vector<float>>be_pressure)
{
	/*  参数说明：
	*   check_point_list：检测点列表（结点名称，所在类别（不需要））
	*   leak_inf：欲设置爆管管道的相关信息，管道名称，时间，所在类别
	*   be_pressure：正常状态下检测点压力各时刻压力值
	*/

	std::cout << endl;
	std::cout << "Start the random pipe burst simulation event..." << endl;
	int size_1 = leak_inf.size();
	int size_2 = timestep.size();
	int size_3 = size_1 * size_2;

	//字典，正常状态下各节点结点压力值
	map<string, vector<float>> map_normalpressure;
	for (int i = 0; i < be_pressure[0].size(); i++)
	{
		vector<float> node_pressure;
		for (int j = 0; j < be_pressure.size(); j++)
		{
			node_pressure.push_back(be_pressure[j][i]);
		}
		map_normalpressure[check_point_list[i][0]] = node_pressure;
	}

	
	for (int i = 0; i < leak_inf.size(); i++)
	{
		string out_file = "out/ky4/leakdata/leak_" + leak_inf[i][0] + "_" + leak_inf[i][1] + "_" + leak_inf[i][2] + ".csv";

		ENopen(inputfile, report, outputfile); //打开文件

		ENsettimeparam(EN_DURATION, time_duration);
		ENsettimeparam(EN_HYDSTEP, time_step);
		ENsettimeparam(EN_REPORTSTEP, time_step);


		char* leak_pipe_id = (char*)leak_inf[i][0].data();
		int leak_pipe_index;
		ENgetlinkindex(leak_pipe_id, &leak_pipe_index);
		
		//获取管道两边结点
		int start_node_index, end_node_index;
		ENgetlinknodes(leak_pipe_index, &start_node_index, &end_node_index);
		char start_node_id[16], end_node_id[16];
		ENgetnodeid(start_node_index, start_node_id);
		ENgetnodeid(end_node_index, end_node_id);

		long leak_time = atol(leak_inf[i][2].data());
		int leak_cluster = atol(leak_inf[i][1].data());

		ENopenH();
		ENinitH(0);
		long t = 0, tstep = 0;
		long next_time = 0;
		long t_check = 0;
		int x = 0;
		vector<vector<float>>leak_pressure;
		do
		{
			//控制爆管的布尔变量
			bool if_leak = false;
			float fEmitterCoeff_start = 0;
			float fEmitterCoeff_end = 0;
			if (leak_time == next_time)
			{
				
				if_leak = true;				
				
				//获取管道两边结点正常状态下压力值
				float be_pressure_start, be_pressure_end;
				
				map<string, vector<float>>::iterator it_start;
				map<string, vector<float>>::iterator it_end;
				it_start = map_normalpressure.find(start_node_id);
				it_end = map_normalpressure.find(end_node_id);
				vector<float>be_pressure_start_timelist, be_pressure_end_timelist;//两个结点的各时刻压力值
				if (it_start != map_normalpressure.end()&& it_end != map_normalpressure.end())
				{
					be_pressure_start_timelist = it_start->second;
					be_pressure_end_timelist = it_end->second;
				}
				be_pressure_start = be_pressure_start_timelist[x];
				be_pressure_end = be_pressure_end_timelist[x];

				//计算爆管系数
				if(be_pressure_start > 0) 
				{
					fEmitterCoeff_start = (float)sFlow* pow(be_pressure_start, 0.5);  //计算喷射系数
				}
				if (be_pressure_end > 0)
				{
					fEmitterCoeff_end = (float)sFlow* pow(be_pressure_end, 0.5);  //计算喷射系数
				}
				//设置爆管系数
				ENsetnodevalue(start_node_index, EN_EMITTER, fEmitterCoeff_start);
				ENsetnodevalue(end_node_index, EN_EMITTER, fEmitterCoeff_end);
			}

			ENrunH(&t);
			next_time = t;
			//剔除非时间步序列中的时刻
			if (t != t_check)
			{
				ENnextH(&tstep);
				next_time += tstep;
				continue;
			}
			//获取模拟数值
			vector<float>leak_pressure_temp;
			for (int j = 0; j < check_point_list.size(); j++)
			{
				char* check_id = (char*)check_point_list[j][0].data();
				int check_index;
				ENgetnodeindex(check_id, &check_index);
				float pressure;
				ENgetnodevalue(check_index, EN_PRESSURE, &pressure);
				leak_pressure_temp.push_back(pressure);
			}
			leak_pressure.push_back(leak_pressure_temp);
			if (true == if_leak)
			{
				ENsetnodevalue(start_node_index, EN_EMITTER, 0);
				ENsetnodevalue(end_node_index, EN_EMITTER, 0);
			}
			ENnextH(&tstep);
			t_check += time_step;
			x++;
			next_time += tstep;
		} while (tstep > 0);
		ENcloseH();
		std::printf("%.2lf%%\r", (i * size_2 + x) * 100.0 / size_3);//进度条
		ENclose();
		vector<string> leak_inf;
		leak_inf.push_back(leak_pipe_id);
		leak_inf.push_back(to_string(leak_time));
		leak_inf.push_back(to_string(leak_cluster));
		//out_check_leak(out_file, leak_pressure, leak_inf, check_point_list);
		out_check_leak_for_day(out_file, leak_pressure, check_point_list);
	}
	std::cout << "The random pipe burst simulation event is completed！" << endl;
}

//对管道进行爆管模拟，一次模拟设置多次爆管事件
void Simpipeleak::leak_pipe_simulate_mutl(string out_file,vector<vector<string>> check_point_list, vector<vector<string>> leak_inf, vector<vector<float>>be_pressure, int leak_scale)
{
	/*  参数说明：
	*   check_point_list：检测点列表（结点名称，所在类别（不需要））
	*   leak_inf：欲设置爆管管道的相关信息:管道名称，所在类别，时间
	*   be_pressure：正常状态下检测点压力各时刻压力值
	*/

	std::cout << endl;
	std::cout << "Start the random pipe burst simulation event..." << endl;

	//字典，正常状态下各节点结点压力值
	map<string, vector<float>> map_normalpressure;
	for (int i = 0; i < be_pressure[0].size(); i++)
	{
		vector<float> node_pressure;
		for (int j = 0; j < be_pressure.size(); j++)
		{
			node_pressure.push_back(be_pressure[j][i]);
		}
		map_normalpressure[check_point_list[i][0]] = node_pressure;
	}

	//clock_t this_time;
	//this_time = clock();
	//string str_this_time = to_string(this_time);
	//string out_file = "out/ky4/leakdata/leak_" + str_this_time + ".csv";

	ENopen(inputfile, report, outputfile); //打开文件

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);

	long leak_time;

	ENopenH();
	ENinitH(0);
	long t = 0, tstep = 0;
	long next_time = 0;
	long t_check = 0;
	int x = 0;
	vector<vector<float>>leak_pressure;
	int i_leakinf = 0;
	do
	{

		//控制爆管的布尔变量
		bool if_leak = false;
		float fEmitterCoeff_start = 0;
		float fEmitterCoeff_end = 0;


		vector<int>already_leak_pipe;
		if (i_leakinf < leak_inf.size())
		{
			leak_time = atol(leak_inf[i_leakinf][2].data());
		}
		while (leak_time == next_time)
		{
			if_leak = true;

			char* leak_pipe_id = (char*)leak_inf[i_leakinf][0].data();
			int leak_pipe_index;
			ENgetlinkindex(leak_pipe_id, &leak_pipe_index);

			//获取管道两边结点
			int start_node_index, end_node_index;
			ENgetlinknodes(leak_pipe_index, &start_node_index, &end_node_index);
			char start_node_id[16], end_node_id[16];
			ENgetnodeid(start_node_index, start_node_id);
			ENgetnodeid(end_node_index, end_node_id);

			//获取管道两边结点正常状态下压力值
			float be_pressure_start, be_pressure_end;

			map<string, vector<float>>::iterator it_start;
			map<string, vector<float>>::iterator it_end;
			it_start = map_normalpressure.find(start_node_id);
			it_end = map_normalpressure.find(end_node_id);
			vector<float>be_pressure_start_timelist, be_pressure_end_timelist;//两个结点的各时刻压力值
			if (it_start != map_normalpressure.end() && it_end != map_normalpressure.end())
			{
				be_pressure_start_timelist = it_start->second;
				be_pressure_end_timelist = it_end->second;
			}
			be_pressure_start = be_pressure_start_timelist[x];
			be_pressure_end = be_pressure_end_timelist[x];

			//计算爆管系数
			if (be_pressure_start > 0)
			{
				fEmitterCoeff_start = (float)sFlow* pow(be_pressure_start, 0.5) * leak_scale;  //计算喷射系数
			}
			if (be_pressure_end > 0)
			{
				fEmitterCoeff_end = (float)sFlow* pow(be_pressure_end, 0.5) * leak_scale;  //计算喷射系数
			}
			//设置爆管系数
			ENsetnodevalue(start_node_index, EN_EMITTER, fEmitterCoeff_start);
			already_leak_pipe.push_back(start_node_index);
			ENsetnodevalue(end_node_index, EN_EMITTER, fEmitterCoeff_end);
			already_leak_pipe.push_back(end_node_index);

			if (++i_leakinf >= leak_inf.size())
			{
				break;
			}
			leak_time = atol(leak_inf[i_leakinf][2].data());
		}


		ENrunH(&t);
		next_time = t;
		//剔除非时间步序列中的时刻
		if (t != t_check)
		{
			ENnextH(&tstep);
			next_time += tstep;
			continue;
		}
		//获取模拟数值
		vector<float>leak_pressure_temp;
		for (int j = 0; j < check_point_list.size(); j++)
		{
			char* check_id = (char*)check_point_list[j][0].data();
			int check_index;
			ENgetnodeindex(check_id, &check_index);
			float pressure;
			ENgetnodevalue(check_index, EN_PRESSURE, &pressure);
			leak_pressure_temp.push_back(pressure);
		}
		leak_pressure.push_back(leak_pressure_temp);
		if (true == if_leak)
		{
			for (int y = 0; y < already_leak_pipe.size(); y++)
			{
				ENsetnodevalue(already_leak_pipe[y], EN_EMITTER, 0);
			}
			already_leak_pipe.clear();
		}
		ENnextH(&tstep);
		t_check += time_step;
		std::printf("%.2lf%%\r", x * 100.0 / (time_duration / time_step));//进度条
		x++;
		next_time += tstep;
	} while (tstep > 0);
	ENcloseH();
	ENclose();

	out_check_leak_for_day(out_file, leak_pressure, check_point_list);
	std::cout << "The random pipe burst simulation event is completed！" << endl;
}


//一次性输出所有模拟结果数据
void Simpipeleak::out_check_leak(string file_path, vector<vector<float>> pressure, vector<string> leak_inf, vector<vector<string>> check_point_list)
{
	ofstream file;
	file.open(file_path, ios::out);

	int len = timestep.size();
	file << "leak_pipe," << "leak_time," << "pipe_cluster," << "check_id,";
	for (int i = 0; i < len - 1; i++)
	{
		file << timestep[i] << ",";
	}
	file << timestep[len - 1] << endl;

	for (int i = 0; i < check_point_list.size(); i++)
	{
		file << leak_inf[0] << "," << leak_inf[1] << "," << leak_inf[2] << "," << check_point_list[i][0] << ",";
		for (int j = 0; j < len; j++)
		{
			file << pressure[j][i] << ",";
		}
		file << endl;
	}

	file.close();

}

//将模拟结果按每一天作为一个文件输出
void Simpipeleak::out_check_leak_for_day(string file_path, vector<vector<float>> pressure, vector<vector<string>> check_point_list)
{
	file_path.erase(file_path.end() - 1);
	file_path.erase(file_path.end() - 1);
	file_path.erase(file_path.end() - 1);
	file_path.erase(file_path.end() - 1);//

	LPCSTR   path = file_path.c_str();
	CreateDirectory(path, NULL); //如果文件夹FileManege不存在，则创建。
	

	int num_day = time_duration / 86400;

	int num_day_setp = 86400 / time_step;

	for (int i = 0; i < num_day; i++)
	{
		string day_file = file_path + "/" + to_string(i + 1)+"day" + ".csv";
		ofstream file;
		file.open(day_file, ios::out);
		file << "check_id,";
		for (int t = 0; t < num_day_setp - 1; t++)
		{
			file << time_step * t << ",";
		}
		file << (num_day_setp - 1) * time_step << endl;

		for (int x = 0; x < check_point_list.size(); x++)
		{
			file << check_point_list[x][0] << ",";
			for (int y = 0; y < num_day_setp; y++)
			{
				file << pressure[i * num_day_setp + y][x] << ",";
			}
			file << endl;
		}
		file.close();
	}
	
}


//随机获取指定数量的爆管设置信息（爆管管道ID，类别，时间），输出至CSV文件，所得文件在get_burstdata（...）函数中使用
void Simpipeleak::random_leak_node(char * pipe_clusterfile, char* out_leak_inf, int number = 1000)
{
	cout << "Randomly set " << number << " pipe burst events..." << endl;
	//读取管道聚类信息CSV文件
	ifstream infile_pipe(pipe_clusterfile, ios::in);
	string linestr_pipe;
	vector<vector<string>> pipe_infarray;   //存储csv数据，二维数组
	while (getline(infile_pipe, linestr_pipe))
	{
		stringstream ss(linestr_pipe);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//按逗号分隔
			linearray.push_back(str);
		pipe_infarray.push_back(linearray);
	}

	pipe_infarray.erase(pipe_infarray.begin());//删除第一个元素

	map<string, int> map_pipecluster;

	for (int i = 0; i < pipe_infarray.size(); i++)
	{
		map_pipecluster[pipe_infarray[i][0]] = stoi(pipe_infarray[i][1]);
	}

	ENopen(inputfile, report, outputfile); //打开文件
	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);


	int pipe_size = pipe_infarray.size();
	int time_size = timestep.size();

	srand((unsigned int)time(0));
	vector<int> pipe_index_list;  //从1开始
	vector<int> time_index_list;  //从0开始

	for (int i = 0; i < number; i++) {

		int temp_pipe = rand() % pipe_size;
		int temp_time = rand() % time_size;

		pipe_index_list.push_back(temp_pipe);
		time_index_list.push_back(temp_time);

	}

	sort(time_index_list.begin(), time_index_list.end());//升序

	vector<long> leak_time;
	vector<string> leak_pipe;
	vector<int> leak_cluster;
	for (int i = 0; i < number; i++)
	{
		leak_time.push_back(time_index_list[i] * time_step);
		string temp_id, leak_cluster_temp;
		temp_id = pipe_infarray[pipe_index_list[i]][0];
		leak_pipe.push_back(temp_id);
		
		leak_cluster_temp = pipe_infarray[pipe_index_list[i]][1];
		leak_cluster.push_back(stoi(leak_cluster_temp));
		
	}

	ofstream file;
	file.open(out_leak_inf, ios::out);
	file << "PipeID," << "Cluster," << "Time" << endl;
	for (int i = 0; i < number; i++)
	{
		file << leak_pipe[i] << "," << leak_cluster[i] << "," << leak_time[i] << endl;
	}
	file.close();
	cout << "Successfully set up a random pipe burst event!" << endl;
}

//获取管网模型的时间模式
void Simpipeleak::gettimepattern()
{
	ENopen(inputfile, report, outputfile); //打开文件

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);

	char patternid[16];
	int patternlen;
	bool ifwhile = true;
	vector<string>timepatternid;
	vector<int>timepatternlen;
	int patternnumber = 1;
	do
	{
		ifwhile = true;
		if (ENgetpatternid(patternnumber, patternid)==0)
		{
			timepatternid.push_back(patternid);
			ENgetpatternlen(patternnumber, &patternlen);
			timepatternlen.push_back(patternlen);
			patternnumber++;
		}
		else
		{
			ifwhile = false;
		}
	} while (ifwhile == true);
	patternnumber--;



	cout << "输出时间模式信息：" << endl;
	cout << "该管网模型共有" << patternnumber << "种时间模式：" << endl;
	cout << "序列  ID  时段数" << endl;
	for (int i = 0; i < timepatternid.size(); i++)
	{
		cout << i + 1 << ": " << timepatternid[i]<<" "<< timepatternlen[i] << ";" << endl;
	}
	cout << endl;

	long patternstep;
	long patternstart;
	ENgettimeparam(EN_PATTERNSTEP, &patternstep);
	ENgettimeparam(EN_PATTERNSTART, &patternstart);
	cout << "时间模式步长为：" << patternstep << "s" << endl;
	cout << "时间模式起始时间为：" << patternstart << "s" << endl << endl;


	cout << "输出时间模式乘子：" << endl;
	cout << "序号 ID 时段序号-乘子" << endl;
	for (int i = 0; i < timepatternid.size(); i++)
	{
		for (int j = 0; j < timepatternlen[i]; j++)
		{
			float patternvalue;
			ENgetpatternvalue(i+1, j+1, &patternvalue);
			cout << i+1 << ": " << timepatternid[i] << " " << j+1 << "-" << patternvalue << endl;
		}
	}

	ENclose();
}

//对所有节点更换时间模式
void Simpipeleak::setnodetimepattern(int pattern_index)
{
	ENopen(inputfile, report, outputfile); //打开文件

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);

	for (int i = 1; i <= node_num; i++)
	{
		ENsetnodevalue(i, EN_NODECOUNT, pattern_index);
	}

}

//从文件中导入正常压力值
void Simpipeleak::load_normal_pressure_data(string pressure_file)
{
	cout << "Imported  normal pressure data from file..." << endl;
	ifstream infile_pressure(pressure_file, ios::in);
	string linestr_pressure;
	vector<string> list_check_point;
	vector<vector<float>> normal_pressure_pos;   //存储csv数据，二维数组
	int row = 1;//行
	while (getline(infile_pressure, linestr_pressure))
	{
		if (row == 1)
		{
			row++;
			continue;
		}
		stringstream ss(linestr_pressure);
		string str;
		vector<float> line_pressure;
		int cal = 1;//列
		while (getline(ss, str, ','))//按逗号分隔
		{
			if (cal == 1)
			{
				list_check_point.push_back(str);//读取并记录检测点ID
			}
			else
			{
				line_pressure.push_back(stof(str));
			}
			cal++;
		}
		normal_pressure_pos.push_back(line_pressure);
		row++;
	}
	
	vector<vector<float>> normal_pressure;
	for (int i = 0; i < normal_pressure_pos[1].size(); i++)
	{
		vector<float> normal_pressure_time;
		for (int j = 0; j < normal_pressure_pos.size(); j++)
		{
			normal_pressure_time.push_back(normal_pressure_pos[j][i]);
		}
		normal_pressure.push_back(normal_pressure_time);
	}

	before_pressure= normal_pressure;
	cout << "Imported  normal pressure data from file successfully!" << endl;
}





