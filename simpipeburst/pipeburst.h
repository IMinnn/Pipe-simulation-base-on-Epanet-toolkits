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
using namespace std;


#define sFlow 0.5       //��������ϵ��
#define scale_pressure 100      //ѹ���仯����ϵ��



//�ж��ļ��Ƿ����
bool exists_test0(const string& name) {
	ifstream f(name.c_str());
	return f.good();
}

//�ж�������Ƿ��ظ�
//bool ifrepetition(int num, vector<int> list);

//�����������
//vector<int> random_list(vector<string> temp_cluster);


class pipeburst
{
public:
		
	int node_num;              //�����ܽ����������ˮ�ص�
	long time_step;           //ˮ��ģ��ʱ�䲽����Ĭ��3600s
	long time_duration;       //ģ������ʱ��Ĭ��24h
	vector<string> node_id;    //���н��ID
	vector<int> node_type;    //���н������
	vector<long> timestep;     //��ʱģ��ʱ���
	vector<vector<float>> before_pressure; //����״̬��ˮ��ģ�����н��ѹ��ֵ,λ��+1=index
	vector<vector<float>> after_pressure;  //����״̬��ˮ��ģ�����н��ѹ��ֵ
	vector<vector<float>> change_pressure; //����ǰ��ѹ���仯����ѹ�����
	vector<string> normal_nodeid; //�޳��쳣���ID
	vector<int> normal_nodeindex; //�޳����index
	vector<vector<double>> nodexy;  //�������ֵ

	pipeburst(char* inputFile);
	~pipeburst();
	void initialize();//��ȡ���������Ϣ
	void sim_before_delay();   //������ʱģ��
	void sim_after_delay(bool if_log, char* log_file);    //����ģ��
	void out_file(char * file, vector<vector<float>> pressure);//��ѹ��ֵ����д��CSV�ļ���
	void pressure_change(vector<vector<float>> be_pressure,vector<vector<float>> af_pressure); //����ѹ����Ϣ
	void set_timeparam(long step, long duration);
	void get_nodexy();
	void print_nodexy(char * file_path);
	void outfile_nodexy(char * out_file, char * marx_file);
	void random_leak_sim(char* cluster_file, char * random_file);
	/*�����ṩ�ĸ���������ڵ��б����©��ģ��,����ֵΪ����Ƕ�׵��ֵ䣺
	*Z1��{ ID1:{P1��P2��...}, ID2:{P1��P2��...}, ... }�� Z2��{ ID1:{P1��P2��...}, ID2:{P1��P2��...}, ... }
	*Z:�����ţ� ID���ڵ�ID�� Pi:i��i����ʱ�̽ڵ�ѹ��ֵ 
	*/
	map<int, map<string, vector<vector<float>>>> random_leak(map<int, vector<string>> leak_list);
	void all_leak_sim(char * cluster_file, char * testdata_file);
	vector<int> random_list(vector<string> temp_cluster); //�����������
	bool ifrepetition(int num, vector<int> list); //�ж�������Ƿ��ظ�

	void get_change__matrix(char * cluster_file, char * out_matrix_file);

	vector<vector<float>> ndoe_normal_sim(vector<string> node_list_);
	vector<vector<vector<float>>> node_leak_sim(vector<string> node_list_, vector<vector<float>> normal_pressure);

	void get_test(char * file);

	void out_check_normal(char * file_path, vector<vector<float>> pressure, vector<string> node_list_, vector<int> node_cluster_);

	void one_point_muti_time(vector<vector<string>>check_point_list, vector<vector<string>>leak_inf, vector<vector<float>>be_pressure);

	//vector<vector<float>> check_point_normal_sim(vector<string> node_list_);
private:
	char *outputfile;          //����������ļ����ɺ���
	char *inputfile;           //�����ļ�
	char *report;              //�����ļ�
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
		 report = "report.txt", outputfile = "";        //����
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

//��ȡ�����������
void pipeburst::initialize()
{
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return;
	}
	char node_id_temp[16];                      //�ڵ�ID
	int node_type_temp;                         //�ڵ�����
	ENopen(inputfile, report, outputfile);

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);
	ENgetcount(EN_NODECOUNT, &node_num);		 //��ȡ�ڵ���
	for (int i = 1; i <= node_num; i++)
	{
		ENgetnodeid(i, node_id_temp);           //��ȡ���ID
		ENgetnodetype(i, &node_type_temp);      //��ȡ�������
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
	std::cout << "��ʼ����ˮ��ģ��..." << endl;
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return;
	}

	ENopen(inputfile, report, outputfile); //���ļ�
	
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
		printf("%.2lf%%\r", i * 100.0 / node_num);//������
	}
	std::cout << endl;
	ENclose();
	std::cout << "sim_before_delay succeeded!" << endl;
}

void pipeburst::sim_after_delay(bool if_log=false,char* log_file=NULL)
{
	std::cout << "��ʼ����ˮ��ģ��..." << endl;
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return;
	}
	ofstream file_log;
	if(if_log) file_log.open(log_file, ios::out);
	ENopen(inputfile, report, outputfile); //���ļ�

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
			if (before_pressure[i - 1][j] < 0)//�ж�ѹ��ֵ�Ƿ�Ϊ��,��Ϊ����ѹ��ֵ����
			{
				fEmitterCoeff = 0;
				if (if_log) file_log << "node:" << node_id[i - 1] << " ,time:" << t << "ʱ��ģ��ѹ��Ϊ��ֵ" << endl;
			}
			else fEmitterCoeff = (float)sFlow* pow(before_pressure[i - 1][j], 0.5);  //��������ϵ��
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
		printf("%.2lf%%\r", i * 100.0 / node_num);//������
	}
	std::cout << endl;
	ENclose();
	if (if_log) file_log.close();
	std::cout << "sim_after_delay succeeded!" << endl;
}

void pipeburst::pressure_change(vector<vector<float>> be_pressure, vector<vector<float>> af_pressure)
{
	cout << "��ʼ����ѹ�����..." << endl;
	int col1, col2; //�д�С
	int row1, row2; //�д�С

	

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
			std::cout << "node��" << node_id[i] << "'s data is false! skip it!" << endl;
			
			continue;
		}
		for (int j = 0; j < col1; j++)
		{
			float del;
			if (be_pressure[i][j] == 0)//��ʼѹ��ֵΪ0������©��ʽ����õ���ѹ��ֵҲΪ0
				del = af_pressure[i][j];
			del = (be_pressure[i][j] - af_pressure[i][j])/fabs(be_pressure[i][j]);
			pressure_temp.push_back(del*scale_pressure);
		}
		change_pressure.push_back(pressure_temp);
		normal_nodeid.push_back(node_id[i]);
	}
	
	cout << "����ѹ��������..." << endl;
}

void pipeburst::get_nodexy()
{
	if (!exists_test0(inputfile))
	{
		std::cout << "file no exist" << endl;
		return;
	}
	ENopen(inputfile, report, outputfile); //���ļ�
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
				std::cout << "node��" << node_id[i] << "'s data is false! skip it!" << endl;
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
	ENopen(inputfile, report, outputfile); //���ļ�
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
		marx_id[ii] = '\0';                  //��ӽ�����

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

void pipeburst::random_leak_sim(char * cluster_file, char * random_file)
{
	//��ȡ������CSV�ļ�
	ifstream infile(cluster_file, ios::in);
	string linestr;
	vector<vector<string>> strarray;   //�洢csv���ݣ���ά����
	while (getline(infile, linestr))
	{
		//cout << linestr << endl;
		stringstream ss(linestr);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//�����ŷָ�
			linearray.push_back(str);
		strarray.push_back(linearray);
	}


	vector<string> node_id;            //��Žڵ�ID
	vector<int> node_cluster;          //���������
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
	
	map<int, vector<string>> cluster_result;  //�������ֵ� {�����ţ�[id1,id2,...],...}

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
	map<int, vector<string>> leak_list;    //��Ҫ����©��ģ��Ľڵ�ID�б��������ھ������
	for (it = cluster_result.begin(); it != cluster_result.end(); it++)
	{
		int cluster_value = it->first;            //��ȡ��ǰ������
		vector<string> temp_cluster = it->second; //��ȡ��ǰ����������нڵ�
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

	//�������ģ��ѹ��
	//map<int, map<string, vector<float>>> random_pressure_change;
	
	ofstream file;
	file.open(random_file, ios::out);
	file << "leak_area_number," << "ID,";
	for (int i = 0; i < timestep.size(); i++)
	{
		if(i< timestep.size()-1) file << timestep[i] << ",";
		else file << timestep[i] << endl;
	}

	map<int, map<string, vector<vector<float>>>>::iterator it_leak_zone;
	for (it_leak_zone = double_pressure_random.begin(); it_leak_zone != double_pressure_random.end(); it_leak_zone++)
	{
		int leak_number = it_leak_zone->first;             //��ȡ��ǰ©���������
		map<string, vector<vector<float>>> node_inf_list = it_leak_zone->second;    //��ȡ��ǰ����ڵ���Ϣ�ֵ�

		map<string, vector<vector<float>>>::iterator it_node_inf_list;
		for (it_node_inf_list = node_inf_list.begin(); it_node_inf_list != node_inf_list.end(); it_node_inf_list++)
		{
			string node_id = it_node_inf_list->first;             //��ȡ��ǰ�ڵ�ID
			vector<vector<float>> node_inf = it_node_inf_list->second;    //��ȡ��ǰ�ڵ�ѹ����Ϣ
			//vector<float> random_pressure_change;
			file << leak_number << "," << node_id << ",";
			for (int i = 0; i < node_inf.size(); i++)
			{
				//random_pressure_change.push_back(node_inf[i][1] - node_inf[i][0]);
				if (i < node_inf.size() - 1) file << node_inf[i][1] - node_inf[i][0] << ",";
				else file << node_inf[i][1] - node_inf[i][0] << endl;
			}			
		}
	}
	file.close();
	//debug...
	int k = 0;
	k++;
}

/*�����ṩ�ĸ���������ڵ��б����©��ģ��,����ֵΪ����Ƕ�׵��ֵ䣺
*Z1��{ ID1:{P1��P2��...}, ID2:{P1��P2��...}, ... }�� Z2��{ ID1:{P1��P2��...}, ID2:{P1��P2��...}, ... }
*Z:�����ţ� ID���ڵ�ID�� Pi:i��i����ʱ�̽ڵ�ѹ��ֵ
*/
map<int, map<string, vector<vector<float>>>> pipeburst::random_leak(map<int, vector<string>> leak_list)
{
	map<int, map<string, vector<vector<float>>>> after_pressure_random_zone;
	if (!exists_test0(inputfile))
	{
		std::cout << "inp file no exist" << endl;
		return after_pressure_random_zone;
	}
	ENopen(inputfile, report, outputfile); //���ļ�

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);
	
	float fEmitterCoeff;

	map<int, vector<string>>::iterator it;
	for (it = leak_list.begin(); it != leak_list.end(); it++)
	{
		int cluster_value = it->first;              //��ȡ��ǰ������
		vector<string> temp_leak_list = it->second; //��ȡ���������Ҫ����©��Ľڵ�ID	
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
				if (before_pressure[node_index - 1][j] < 0)//�ж�ѹ��ֵ�Ƿ�Ϊ��,��Ϊ����ѹ��ֵ����
				{
					fEmitterCoeff = 0;
					//if (if_log) file_log << "node:" << node_id[i - 1] << " ,time:" << t << "ʱ��ģ��ѹ��Ϊ��ֵ" << endl;
				}
				else fEmitterCoeff = (float)sFlow* pow(before_pressure[node_index - 1][j], 0.5);  //��������ϵ��
				
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
			printf("%.2lf%%\r", i * 100.0 / node_num);//������
		}
	
		after_pressure_random_zone[cluster_value] = after_pressure_random_node;
	}
	
	return after_pressure_random_zone;
}

//���ɲ������ݣ�ȫ�ڵ㣩
void pipeburst::all_leak_sim(char * cluster_file, char * testdata_file)
{
	//��ȡ������CSV�ļ�
	ifstream infile(cluster_file, ios::in);
	string linestr;
	vector<vector<string>> strarray;   //�洢csv���ݣ���ά����
	while (getline(infile, linestr))
	{
		//cout << linestr << endl;
		stringstream ss(linestr);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//�����ŷָ�
			linearray.push_back(str);
		strarray.push_back(linearray);
	}


	vector<string> node_id;            //��Žڵ�ID
	vector<int> node_cluster;          //���������
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
	
	map<int, vector<string>> cluster_result;  //�������ֵ� {�����ţ�[id1,id2,...],...}

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

	map<int, map<string, vector<vector<float>>>> double_pressure_random;
	double_pressure_random = random_leak(cluster_result);

	ofstream file;
	file.open(testdata_file, ios::out);
	file << "leak_area_number," << "ID,";
	for (int i = 0; i < timestep.size(); i++)
	{
		if (i < timestep.size() - 1) file << timestep[i] << ",";
		else file << timestep[i] << endl;
	}

	map<int, map<string, vector<vector<float>>>>::iterator it_leak_zone;
	for (it_leak_zone = double_pressure_random.begin(); it_leak_zone != double_pressure_random.end(); it_leak_zone++)
	{
		int leak_number = it_leak_zone->first;             //��ȡ��ǰ©���������
		map<string, vector<vector<float>>> node_inf_list = it_leak_zone->second;    //��ȡ��ǰ����ڵ���Ϣ�ֵ�

		map<string, vector<vector<float>>>::iterator it_node_inf_list;
		for (it_node_inf_list = node_inf_list.begin(); it_node_inf_list != node_inf_list.end(); it_node_inf_list++)
		{
			string node_id = it_node_inf_list->first;             //��ȡ��ǰ�ڵ�ID
			vector<vector<float>> node_inf = it_node_inf_list->second;    //��ȡ��ǰ�ڵ�ѹ����Ϣ
			//vector<float> random_pressure_change;
			file << leak_number << "," << node_id << ",";
			for (int i = 0; i < node_inf.size(); i++)
			{
				//random_pressure_change.push_back(node_inf[i][1] - node_inf[i][0]);
				if (i < node_inf.size() - 1) file << (node_inf[i][1] - node_inf[i][0])/fabs(node_inf[i][1])*100 << ",";
				else file << (node_inf[i][1] - node_inf[i][0]) / fabs(node_inf[i][1])*100 << endl;
			}
		}
	}
	file.close();
}

//�����������
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
//�ж�������Ƿ��ظ�
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

void pipeburst::get_change__matrix(char * cluster_file, char * out_matrix_file)
{
	//��ȡ������CSV�ļ�
	ifstream infile(cluster_file, ios::in);
	string linestr;
	vector<vector<string>> strarray;   //�洢csv���ݣ���ά����
	while (getline(infile, linestr))
	{
		stringstream ss(linestr);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//�����ŷָ�
			linearray.push_back(str);
		strarray.push_back(linearray);
	}

	vector<string> node_list_;
	map<string, int> node_cluster;
	
	ofstream file;
	file.open(out_matrix_file, ios::out);
	file << "cluster,ID";
	for (int i = 1; i < strarray.size(); i++)
	{
		node_list_.push_back(strarray[i][0]);
		node_cluster[strarray[i][0]] = stoi(strarray[i][1]);
		file << strarray[i][0] << ",";
	}
	file << endl;

	vector<vector<float>> normal_pressure;
	normal_pressure=ndoe_normal_sim(node_list_);
	vector<float> mean_1;
	for (int i = 0; i < node_list_.size(); i++)
	{
		float sum_alltime = 0.0;
		for (int j = 0; j < timestep.size(); j++)
		{
			sum_alltime += normal_pressure[i][j];
		}
		float mean_temp = sum_alltime / timestep.size();
		mean_1.push_back(mean_temp);
	}

	vector<vector<vector<float>>> leak_pressure_for_one;
	 vector<float> pressure_for_onenode;
	 leak_pressure_for_one = node_leak_sim(node_list_, normal_pressure);
	
	 vector<vector<float>> change_matrix;

	 for (int i = 0; i < node_list_.size(); i++)
	 {
		 file << strarray[i+1][1] << "," << node_list_[i] << ",";
		 vector<float> change_for_one;
		 for (int j = 0; j < node_list_.size(); j++)
		 {
			 float sum_alltime = 0.0;
			 for (int k = 0; k < timestep.size(); k++)
			 {
				 sum_alltime += leak_pressure_for_one[i][k][j];
			 }
			 float mean_2 = sum_alltime / timestep.size();

			 float change_temp = mean_1[j] - mean_2;
			 //float change_temp = (mean_1[j] - mean_2)/fabs(mean_1[j]);
			 if (j < node_list_.size() - 1)
			 {
				 file << change_temp << ",";
			 }
			 else
			 {
				 file << change_temp << endl;
			 }
			 change_for_one.push_back(change_temp);
		 }
		 change_matrix.push_back(change_for_one);
	 }

	 file.close();

	 int o = 0;
	 o++;

}

vector<vector<float>> pipeburst::ndoe_normal_sim(vector<string> node_list_)
{

	vector<vector<float>> normal_pressure;
	std::cout << "��ʼ����ˮ��ģ��..." << endl;


	ENopen(inputfile, report, outputfile); //���ļ�

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);

	for (int i = 0; i < node_list_.size(); i++)
	{
		int node_index;
		string node_id = node_list_[i];
		char* node_id_ = (char*)node_id.data();
		ENgetnodeindex(node_id_, &node_index);
		ENopenH();
		ENinitH(0);
		long t, tstep;
		long t_check = 0;
		vector<float> pressure_temp;
		do
		{
			ENrunH(&t);
			if (t != t_check)
			{
				ENnextH(&tstep);
				continue;
			}
			float pressure;
			ENgetnodevalue(node_index, EN_PRESSURE, &pressure);
			pressure_temp.push_back(pressure);
			ENnextH(&tstep);
			t_check += time_step;
		} while (tstep > 0);
		ENcloseH();
		normal_pressure.push_back(pressure_temp);
		printf("%.2lf%%\r", (i+1) * 100.0 / node_list_.size());//������
	}
	std::cout << endl;
	ENclose();
	std::cout << "sim_before_delay succeeded!" << endl;
	return normal_pressure;
}

vector<vector<vector<float>>> pipeburst::node_leak_sim(vector<string> node_list_, vector<vector<float>> normal_pressure)
{
	ENopen(inputfile, report, outputfile); //���ļ�

	ENsettimeparam(EN_DURATION, time_duration);
	ENsettimeparam(EN_HYDSTEP, time_step);
	ENsettimeparam(EN_REPORTSTEP, time_step);

	float fEmitterCoeff;
	int size_ndoe = node_list_.size();
	int size_time = timestep.size();
	int size_para = size_ndoe * size_time;
	vector<vector<vector<float>>> pressure_node_for_allone;//pressure_node_for_allone[allnode][timestep][allnode]
	for (int i = 0; i < size_ndoe; i++)
	{
		int node_index;
		string node_id = node_list_[i];
		char* node_id_ = (char*)node_id.data();
		ENgetnodeindex(node_id_, &node_index);

		ENopenH();
		ENinitH(0);
		long t = 0, tstep;
		long t_check = 0;
		int j = 0;
		vector<vector<float>> one_node_pressure;
		do
		{
			printf("%.3lf%%\r", (i * size_time + j) * 100.0 / size_para);//������
			if (normal_pressure[i][j] < 0)//�ж�ѹ��ֵ�Ƿ�Ϊ��,��Ϊ����ѹ��ֵ����
			{
				fEmitterCoeff = 0;
			}
			else fEmitterCoeff = (float)sFlow* pow(normal_pressure[i][j], 0.5);  //��������ϵ��

			ENsetnodevalue(node_index, EN_EMITTER, fEmitterCoeff);
			ENrunH(&t);
			if (t != t_check)
			{
				ENnextH(&tstep);
				continue;
			}
			vector<float> pressure_temp;
			for (int k = 0; k < size_ndoe; k++)
			{
				float pressure;
				int node_index_temp;
				string node_id_temp = node_list_[k];
				char* node_id_temp_ = (char*)node_id_temp.data();
				ENgetnodeindex(node_id_temp_, &node_index_temp);
				ENgetnodevalue(node_index_temp, EN_PRESSURE, &pressure);
				pressure_temp.push_back(pressure);
			}						
			ENsetnodevalue(node_index, EN_EMITTER, 0);
			one_node_pressure.push_back(pressure_temp);
			ENnextH(&tstep);
			t_check += time_step;
			j++;
		} while (tstep > 0);
		ENcloseH();
		pressure_node_for_allone.push_back(one_node_pressure);
	}

	return pressure_node_for_allone;
}

void pipeburst::get_test(char * file)
{
	//��ȡ������CSV�ļ�
	ifstream infile(file, ios::in);
	string linestr;
	vector<vector<string>> strarray;   //�洢csv���ݣ���ά����
	while (getline(infile, linestr))
	{
		stringstream ss(linestr);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//�����ŷָ�
			linearray.push_back(str);
		strarray.push_back(linearray);
	}

	strarray.erase(strarray.begin());//ɾ����һ��Ԫ��

	vector<string> node_list_; 
	vector<int> node_cluster_;
	map<string, int> node_cluster;

	for (int i = 0; i < strarray.size(); i++)
	{
		node_list_.push_back(strarray[i][0]);
		node_cluster_.push_back(stoi(strarray[i][1]));
		node_cluster[strarray[i][0]] = stoi(strarray[i][1]);
	}

	vector<vector<float>> normal_pressure;
	normal_pressure = ndoe_normal_sim(node_list_);

	//char* file_check_point_normal = "out/ky4/check_normal.csv";
	//out_check_normal(file_check_point_normal, normal_pressure, node_list_, node_cluster_); //�����CSV�ļ�


	//��ȡ������CSV�ļ�
	char * in_file_1point_muti_time = "out/ky4/1point_muti_time.csv";
	ifstream infile_1(in_file_1point_muti_time, ios::in);
	string linestr_1;
	vector<vector<string>> strarray_1;   //�洢csv���ݣ���ά����
	while (getline(infile_1, linestr_1))
	{
		stringstream ss(linestr_1);
		string str;
		vector<string> linearray;
		while (getline(ss, str, ','))//�����ŷָ�
			linearray.push_back(str);
		strarray_1.push_back(linearray);
	}

	strarray_1.erase(strarray_1.begin());//ɾ����һ��Ԫ��
	
	//vector<vector<float>> leak_pressure;
	//normal_pressure = ndoe_normal_sim(node_list_);

	one_point_muti_time(strarray, strarray_1, normal_pressure);
}

void pipeburst::out_check_normal(char * file_path, vector<vector<float>> pressure, vector<string> node_list_, vector<int> node_cluster_)
{
	ofstream file;
	file.open(file_path, ios::out);

	int len = timestep.size();
	file << "Cluster,";
	file << "ID,";
	for (int i = 0; i < len - 1; i++)
	{
		file << timestep[i] << ",";
	}
	file << timestep[len - 1] << endl;

	for (int i = 0; i < node_list_.size(); i++)
	{
		file << node_cluster_[i]<<","<< node_list_[i] << ",";
		for (int j = 0; j < len; j++)
		{
			file << pressure[i][j] << ",";
		}
		file << endl;
	}

	file.close();

	std::cout << " write to file succeeded!" << endl;
}

void pipeburst::one_point_muti_time(vector<vector<string>> check_point_list, vector<vector<string>> leak_inf, vector<vector<float>>be_pressure)
{
	for (int i = 0; i < leak_inf.size(); i++)
	{
		string out_file_1 = "out/ky4/leak_";
		string out_file_2 = leak_inf[i][0];
		string out_file_3 = ".csv";
		string out_file = "out/ky4/leak_" + leak_inf[i][0] + ".csv";

		ofstream file;
		file.open(out_file, ios::out);

		file << "leak_ndoe," << "leak_time," << "if_cluster," << "check_id,";

		for (int k = 0; k < timestep.size() - 1; k++)
		{
			file << timestep[k] << ",";
		}
		file << timestep[timestep.size() - 1] << endl;

		ENopen(inputfile, report, outputfile); //���ļ�

		ENsettimeparam(EN_DURATION, time_duration);
		ENsettimeparam(EN_HYDSTEP, time_step);
		ENsettimeparam(EN_REPORTSTEP, time_step);

		float fEmitterCoeff = 0;;

		char* leak_node = (char*)leak_inf[i][0].data();
		int leak_index;
		ENgetnodeindex(leak_node, &leak_index);
		long leak_time = atol(leak_inf[i][2].data());
		int leak_cluster = atol(leak_inf[i][1].data());

		vector<vector<float>> leak_pressure;

		for (int j = 0; j < check_point_list.size(); j++)
		{
			char* check_id = (char*)check_point_list[j][0].data();
			bool if_cluster = 0;
			if (atol(check_point_list[j][1].data()) == leak_cluster)
			{
				if_cluster = 1;
			}
			file << leak_node << "," << leak_time << "," << if_cluster << "," << check_id << ",";

			int check_index;
			ENgetnodeindex(check_id, &check_index);

			ENopenH();
			ENinitH(0);
			long t = 0, tstep;
			long t_check = 0;
			int x = 0;
			vector<float>leak_pressure_temp;
			do
			{

				if (leak_time == t)
				{

					if (be_pressure[j][x] < 0)//�ж�ѹ��ֵ�Ƿ�Ϊ��,��Ϊ����ѹ��ֵ����
					{
						fEmitterCoeff = 0;
					}
					else fEmitterCoeff = (float)sFlow* pow(be_pressure[j][x], 0.5);  //��������ϵ��
				}
				ENsetnodevalue(leak_index, EN_EMITTER, fEmitterCoeff);
				ENrunH(&t);
				if (t != t_check)
				{
					ENnextH(&tstep);
					continue;
				}
				float pressure;
				ENgetnodevalue(check_index, EN_PRESSURE, &pressure);
				file << pressure << ",";
				ENsetnodevalue(leak_index, EN_EMITTER, 0);
				leak_pressure_temp.push_back(pressure);
				ENnextH(&tstep);
				t_check += time_step;
				x++;
			} while (tstep > 0);
			file << endl;
			ENcloseH();
			leak_pressure.push_back(leak_pressure_temp);
			//printf("%.2lf%%\r", i * 100.0 / node_num);//������
		}

		file.close();
	}

}




