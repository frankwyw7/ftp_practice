#include "Header_processor.h"
#include <cstring>
#include <string>
#include "Session.h"
#include "Server_manager.h"
#include <memory>
#include "help_function.h"

namespace my_ftp
{
	static const int maxlen = 6;
	static const int invalid_num = 4;

	char requests[][maxlen] = { "login", "query", "dowld", "quitt"};

	Header_processor::Header_processor(boost::filesystem::path& path) :path_(path) {};

	Header_processor::~Header_processor() {};

	Request_t Header_processor::parser_header(char* buffer)
	{
		pruntime("parser_header");
		if (buffer == "")
			return Request_t::valid;

		char request[maxlen];
		for (int i = 0; i < maxlen - 1; i++)
			request[i] = buffer[i];
		request[maxlen-1] = '\0';

		//bool equal_flag = false;

		//for (int i = 0; i < invalid_num; i++)
		//{
		//	if (strcmp(request, requests[i]))
		//		equal_flag = true;
		//}

		//if (!equal_flag)
		//	return Request_t::valid;

		if (!strcmp(request, requests[0]))
			return Request_t::login;
		else if (!strcmp(request, requests[1]))
			return Request_t::query;
		else if (!strcmp(request, requests[2]))
			return Request_t::dowld;
		else if (!strcmp(request, requests[3]))
			return Request_t::quitt;
		else
			return Request_t::valid;

	}

	int Header_processor::procees_request(char* ibuffer, std::shared_ptr<Session> session, char* obuffer)
	{
		//std::string tmp;
		pruntime("procees_request");
		switch (Header_processor::parser_header(ibuffer))
		{
		case login:
			//比较后面的用户名与密码
			cmppwd(ibuffer, obuffer);
			break;

		case query:
			query_file(ibuffer, obuffer);
			break;

		case dowld:
			download(ibuffer, session);
			return 1;

		case quitt:
			//产生了调用环路，引用计数为零时，先析构还是执行完本函数后析构
			//session->get_manager()->delete_session(session);
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
		pruntime("cmppwd");
		std::vector<char> vec_char;
		for (int i = 5; (ibuffer[i] != '\r'&&ibuffer[i + 1] != '\n') && ibuffer[i] != '\0'; ++i)
			vec_char.push_back(ibuffer[i]);

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

	int Header_processor::query_file(char* ibuffer, char* obuffer) 
	{
		pruntime("query_file");
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
		return 0;
	};

	int Header_processor::download(char* ibuffer, std::shared_ptr<Session> session)
	{
		pruntime("download");
		std::vector<char> vec_char;
		for (int i = 5; (ibuffer[i] != '\r'&&ibuffer[i + 1] != '\n') && ibuffer[i] != '\0'; ++i)
			vec_char.push_back(ibuffer[i]);
		boost::filesystem::path path(path_);
		std::string tmp(vec_char.begin(), vec_char.end());
		path.append(tmp);
		session->set_downld_flie(path.string());
		return 0;
	};
}