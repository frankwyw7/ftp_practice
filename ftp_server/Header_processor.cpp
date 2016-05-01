#include "Header_processor.h"
#include "Session.h"
#include "Server_manager.h"
#include "../common/help_function.h"
#include <cstring>
#include <string>
#include <memory>
#include <sstream>
#include <cstdlib>

namespace my_ftp
{
	static const int maxlen = 6;
	static const int invalid_num = 4;
	static const int data_begin_pos = 8;

	char requests[][maxlen] = { "login", "query", "dowld", "quitt"};

	Header_processor::Header_processor(boost::filesystem::path& path) :path_(path) {};

	Header_processor::~Header_processor() {};

	Request_t Header_processor::parser_header(char* buffer)
	{
		pruntime("Header_processor parser_header");

		request.decode(buffer);
		int tmp = atoi(request.type);

		return static_cast<Request_t>(tmp);
	}

	//返回1  下载文件
	//返回0  直接发送缓冲区
	//返回-1 退出
	int Header_processor::procees_request(char* ibuffer, std::shared_ptr<Session> session, char* obuffer)
	{
		//std::string tmp;
		pruntime("Header_processor procees_request");
		switch (Header_processor::parser_header(ibuffer))
		{
		case login:
			cmppwd(ibuffer, obuffer);
			break;

		case query:
			query_file(ibuffer, obuffer);
			break;

		case dowld:
			if (download(ibuffer, session) == 1)
			{
				char tmp[] = "00070019file is not exist";
				strcpy(obuffer, tmp);
				return 0;
			}
			return 1;

		case quitt:
			return -1;

		case valid:
		{	
			char tmp[] = "valid";
			strcpy(obuffer, tmp);
		}
			break;

		default:
			break;
		};
		return 0;
	}

	int Header_processor::cmppwd(char* ibuffer, char* obuffer) 
	{
		pruntime("Header_processor cmppwd");
		std::vector<char> vec_char;
		int len = atoi(request.length);
		for (int i = 0; i < len; ++i)
			vec_char.push_back(request.data[i]);

		std::string str(vec_char.begin(), vec_char.end());
		std::string str_pwd;
		boost::filesystem::path tmp_path = path_ / "pwd.txt";
		
		std::string tmp = tmp_path.string();
		std::fstream fs(tmp);
		
		bool flag = false;
		while (std::getline(fs, str_pwd))
		{
			if (str_pwd == str)
			{
				flag = true;
			}
		}
		if (flag)
		{
			char tmp[] = "login ok";
			strcpy(obuffer, tmp);
		}
		else
			return -1;
		return 0;
	};

	//查询FTP文件夹当前包含文件
	int Header_processor::query_file(char* ibuffer, char* obuffer) 
	{
		pruntime("Header_processor query_file");
		std::vector<std::string> vec;
		boost::filesystem::directory_iterator end_iter;

		int obuf_k = 0;
		for (boost::filesystem::directory_iterator ite(path_);
		ite != end_iter; ++ite)
		{
			std::string str = (*ite).path().string();
			for (int i = 0; i < str.size(); ++i)
			{
				obuffer[obuf_k] = str[i];
				++obuf_k;
			}
			obuffer[obuf_k++] = '\n';
		}
		obuffer[obuf_k++] = '\0';
		return 0;
	};
	//fstream打开文件，无法打开返回1
	int Header_processor::download(char* ibuffer, std::shared_ptr<Session> session)
	{
		pruntime("Header_processor download");
		std::vector<char> vec_char;
		int len = atoi(request.length);
		for (int i = 0; i < len; ++i)
			vec_char.push_back(request.data[i]);
		boost::filesystem::path path(path_);
		std::string tmp(vec_char.begin(), vec_char.end());
		path.append(tmp);
		if (!session->set_downld_flie(path.string()))
			return 1;
		return 0;
	};

}