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
using namespace std;


#define sFlow 0.5       //设置喷射系数
#define scale_pressure 100      //压力变化扩大系数



//判断文件是否存在
bool exists_test0(const string& name) {
	ifstream f(name.c_str());
	return f.good();
}

//判断随机数是否重复
//bool ifrepetition(int num, vector<int> list);

//生成随机数组
//vector<int> random_list(vector<string> temp_cluster);


class pipeburst
{
public:
		
	int node_num;              //管网总结点数，包括水池等
	long time_step;           //水力模拟时间步长，默认3600s
	long time_duration;       //模拟总历时，默认24h
	vector<string> node_id;    //所有结点ID
	vector<int> node_type;    //所有结点类型
	vector<long> timestep;     //延时模拟时间戳
	vector<vector<float>> before_pressure; //正常状态下水力模拟所有结点压力值,位置+1=index
	vector<vector<float>> after_pressure;  //爆管状态下水力模拟所有结点压力值
	vector<vector<float>> change_pressure; //爆管前后压力变化矩阵（压变矩阵）
	vector<string> normal_nodeid; //剔除异常结点ID
	vector<int> normal_nodeindex; //剔除结点index
	vector<vector<double>> nodexy;  //结点坐标值

	pipeburst(char* inputFile);
	~pipeburst();
	void initialize();//获取管网相关信息
	void sim_before_delay();   //正常延时模拟
	void sim_after_delay(bool if_log, char* log_file);    //爆管模拟
	void out_file(char * file, vector<vector<float>> pressure);//将压力值数据写入CSV文件中
	void pressure_change_matrix(vector<vector<float>> be_pressure,vector<vector<float>> af_pressure); //计算压变矩阵
	void set_timeparam(long step, long duration);
	void get_nodexy();
	void print_nodexy(char * file_path);
	void outfile_nodexy(char * out_file, char * marx_file);
	void random_leak_sim(char* cluster_file);
	/*根据提供的各区域随机节点列表进行漏损模拟,返回值为两次嵌套的字典：
	*Z1：{ ID1:{P1，P2，...}, ID2:{P1，P2，...}, ... }， Z2：{ ID1:{P1，P2，...}, ID2:{P1，P2，...}, ... }
	*Z:区域编号； ID：节点ID； Pi:i第i步长时刻节点压力值 
	*/
	map<int, map<string, vector<vector<float>>>> random_leak(map<int, vector<string>> leak_list);
	vector<int> random_list(vector<string> temp_cluster); //生成随机数组
	bool ifrepetition(int num, vector<int> list); //判断随机数是否重复
private:
	char *outputfile;          //二进制输出文件，可忽略
	char *inputfile;           //管网文件
	char *report;              //报告文件
};



 pipeburst::pipeburst(char* file)
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

 pipeburst::~pipeburst()
 {
	 std::cout << "pipeburst class delete succeeded!" << endl;
 }

 void pipeburst::set_timeparam(long step, long duration = 86400)
 {
	 time_duration = duration;
	 time_step = step;
 }

//获取管网相关数据
void pipeburst::initialize()
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
	}while(tstep > 0);
	ENcloseH();
	ENclose();
	std::cout << "get information for pipe network succeeded!" << endl;
	int kk = 0;
	kk++;
}

void pipeburst::sim_before_delay()
{
	std::cout << "开始正常水力模拟..." << endl;
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return;
	}

	ENopen(inputfile, report, outputfile); //打开文件
	
	ENsettimeparam(EN_DURATION,time_duration);
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

void pipeburst::sim_after_delay(bool if_log=false,char* log_file=NULL)
{
	std::cout << "开始爆管水力模拟..." << endl;
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return;
	}
	ofstream file_log;
	if(if_log) file_log.open(log_file, ios::out);
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

void pipeburst::pressure_change_matrix(vector<vector<float>> be_pressure, vector<vector<float>> af_pressure)
{
	cout << "开始计算压变矩阵..." << endl;
	int col1, col2; //列大小
	int row1, row2; //行大小

	

	vector<float>size_temp = be_pressure[0];
	col1 = size_temp.size();
	size_temp = af_pressure[0];
	col2 = size_temp.size();
	row1 = be_pressure.size();
	row2 = af_pressure.size();

	if (col1 != col2 || row1 != row2|| col1 == 0 || row1 == 0)
	{
		std::cout << "calculate pressure matrix failed!" << endl;
		return;
	}
	
	for (int i = 0; i < row1; i++)
	{
		vector<float> pressure_temp;
		if (af_pressure[i].size() != be_pressure[i].size()|| af_pressure[i].size()!=timestep.size())
		{
			std::cout << "node：" << node_id[i] << "'s data is false! skip it!" << endl;
			
			continue;
		}
		for (int j = 0; j < col1; j++)
		{
			float del;
			if (be_pressure[i][j] == 0)//初始压力值为0，根据漏损公式计算得到的压力值也为0
				del = af_pressure[i][j];
			del = (be_pressure[i][j] - af_pressure[i][j])/fabs(be_pressure[i][j]);
			pressure_temp.push_back(del*scale_pressure);
		}
		change_pressure.push_back(pressure_temp);
		normal_nodeid.push_back(node_id[i]);
	}
	
	cout << "计算压变矩阵完成..." << endl;
}

void pipeburst::get_nodexy()
{
	if (!exists_test0(inputfile))
	{
		std::cout << "file no exist" << endl;
		return;
	}
	ENopen(inputfile, report, outputfile); //打开文件
	for (int i = 1; i <= node_num; i++)
	{
		double x, y;
		ENgetcoord(i,&x,&y);
		vector<double> nodexy_temp;
		nodexy_temp.push_back(x);
		nodexy_temp.push_back(y);
		nodexy.push_back(nodexy_temp);
	}

	ENclose();
}

void pipeburst::out_file(char * file_path, vector<vector<float>> pressure)
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

void pipeburst::print_nodexy(char * file_path)
{
	ofstream file;
	file.open(file_path, ios::out);
	int len = normal_nodeid.size();
	for (int i = 0; i < len; i++)
	{		
		int index_temp;
		char *nodeid_temp = (char*)normal_nodeid[i].data();
		ENgetnodeindex(nodeid_temp, &index_temp);
		double x, y;
		ENgetcoord(index_temp, &x, &y);
		file << node_id[i] << "," << nodexy[i][0] << "," << nodexy[i][1] << endl;
			
	}
	file.close();
	std::cout << " write to nodexy file succeeded!" << endl;
}

void pipeburst::outfile_nodexy(char * out_file, char * marx_file)
{
	ofstream file;
	file.open(out_file, ios::out);
	ENopen(inputfile, report, outputfile); //打开文件
	ifstream fin(marx_file,ios::in);
	string line;
	vector<string> marx_nodeid;
	int o = 0;
	while (getline(fin, line))
	{
		
		o++;
		marx_nodeid.push_back(line);
		char marx_id[20];
		int ii;
		for (ii = 0; ii < line.length(); ii++)
			marx_id[ii] = line[ii];
		marx_id[ii] = '\0';                  //添加结束符

		cout << line << "," << marx_id << endl;
		int index_temp;
		double x, y;
		ENgetnodeindex(marx_id, &index_temp);
		ENgetcoord(index_temp, &x, &y);
		file << marx_id << "," << x << "," << y << endl;
	}
	





	ENclose();
	fin.close();

	file.close();
	std::cout << " write to nodexy file succeeded!" << endl;
}

void pipeburst::random_leak_sim(char * cluster_file)
{
	//读取聚类结果CSV文件
	ifstream infile(cluster_file, ios::in);
	string linestr;
	vector<vector<string>> strarray;   //存储csv数据，二维数组
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


	vector<string> node_id;            //存放节点ID
	vector<int> node_cluster;          //存放类别序号
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
	
	map<int, vector<string>> cluster_result;  //聚类结果字典 {类别序号：[id1,id2,...],...}

	int len_cluster = node_id.size();
	int temp = node_cluster[0];
	vector<string> temp_map;
	for (int i = 0; i < len_cluster; i++)
	{
		if (node_cluster[i] == temp)
		{
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
	map<int, vector<string>> leak_list;    //需要进行漏损模拟的节点ID列表，及其所在聚类类别
	for (it = cluster_result.begin(); it != cluster_result.end(); it++)
	{
		int cluster_value = it->first;            //获取当前类别序号
		vector<string> temp_cluster = it->second; //获取当前所在类别所有节点
		int temp_number = temp_cluster.size();
		vector<int>leak_list_temp = random_list(temp_cluster);
		vector<string>leak_list_id_temp;
		for (int i = 0; i < leak_list_temp.size(); i++)
		{
			cout << temp_cluster[leak_list_temp[i]] << ", ";
			leak_list_id_temp.push_back(temp_cluster[leak_list_temp[i]]);
		}
		cout << endl;
		leak_list[cluster_value] = leak_list_id_temp;
		leak_list_id_temp.clear();
	}

	map<int, map<string, vector<vector<float>>>> double_pressure_random;
	double_pressure_random = random_leak(leak_list);



	//debug...
	int i = 0;
	i++;
}

/*根据提供的各区域随机节点列表进行漏损模拟,返回值为两次嵌套的字典：
*Z1：{ ID1:{P1，P2，...}, ID2:{P1，P2，...}, ... }， Z2：{ ID1:{P1，P2，...}, ID2:{P1，P2，...}, ... }
*Z:区域编号； ID：节点ID； Pi:i第i步长时刻节点压力值
*/
map<int, map<string, vector<vector<float>>>> pipeburst::random_leak(map<int, vector<string>> leak_list)
{
	map<int, map<string, vector<vector<float>>>> after_pressure_random_zone;
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return after_pressure_random_zone;
	}
	ENopen(inputfile, report, outputfile); //打开文件

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);
	
	float fEmitterCoeff;

	map<int, vector<string>>::iterator it;
	for (it = leak_list.begin(); it != leak_list.end(); it++)
	{
		int cluster_value = it->first;              //获取当前类别序号
		vector<string> temp_leak_list = it->second; //获取此类别中需要进行漏损的节点ID	
		vector<int> temp_leak_list_index;
		map<string, vector<vector<float>>> after_pressure_random_node;
		for (int i = 0; i < temp_leak_list.size(); i++)
		{
			int node_index;
			string node_id = temp_leak_list[i];
			char* node_id_ = (char*)node_id.data();
			ENgetnodeindex(node_id_, &node_index);

			ENopenH();
			ENinitH(0);
			long t = 0, tstep;
			long t_check = 0;
			int j = 0;
			vector<vector<float>> double_pressure_random_node;
			do
			{
				vector<float> double_pressure_random_node_temp;
				if (before_pressure[node_index - 1][j] < 0)//判断压力值是否为负,若为负则压力值不变
				{
					fEmitterCoeff = 0;
					//if (if_log) file_log << "node:" << node_id[i - 1] << " ,time:" << t << "时刻模拟压力为负值" << endl;
				}
				else fEmitterCoeff = (float)sFlow* pow(before_pressure[node_index - 1][j], 0.5);  //计算喷射系数
				
				ENsetnodevalue(node_index, EN_EMITTER, fEmitterCoeff);
				ENrunH(&t);
				if (t != t_check)
				{
					ENnextH(&tstep);
					continue;
				}
				float pressure;
				ENgetnodevalue(node_index, EN_PRESSURE, &pressure);
				ENsetnodevalue(node_index, EN_EMITTER, 0);
				double_pressure_random_node_temp.push_back(pressure);
				double_pressure_random_node_temp.push_back(before_pressure[node_index - 1][j]);
				double_pressure_random_node.push_back(double_pressure_random_node_temp);
				ENnextH(&tstep);
				t_check += time_step;
				j++;
			} while (tstep > 0);
			ENcloseH();
			after_pressure_random_node[node_id] = double_pressure_random_node;
			printf("%.2lf%%\r", i * 100.0 / node_num);//进度条
		}
	
		after_pressure_random_zone[cluster_value] = after_pressure_random_node;
	}
	
	return after_pressure_random_zone;
}
//生成随机数组
vector<int> pipeburst::random_list(vector<string> temp_cluster)
{
	int number = temp_cluster.size();
	int leak_number = 0;
	if (number <= 4)
	{
		leak_number = number;
	}
	else
	{
		leak_number =2 * (int)sqrt(number);

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
//判断随机数是否重复
bool pipeburst::ifrepetition(int num, vector<int> list)
{
	int size = list.size();
	if (size == 0)return false;
	for (int i = 0; i < size; i++)
	{
		if (num == list[i])return true;
	}
	return false;
}


