#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <fstream>
#include <sstream>
#include "../common/help_function.h"
#include "../common/Request.h"



namespace my_ftp
{
	using boost::asio::ip::tcp;
	using namespace std;

	enum { max_length = 4096 };

	static const int command_len = 3;
	static const int command_count = 4;
	static const int user_maxlen = 15;
	static const int pwd_maxlen = 30;
	static const int buf_maxlen = 4096;
	static const int data_start = 8;
	static const int length_start = 4;

	bool socket_not_clear = false;

	char commands[command_count][command_len] = { "lg", "qe", "dl", "qt" };

	void send(char * out_buffer, tcp::socket& socket_)
	{
		if (out_buffer == nullptr)
			return;
		boost::asio::write(socket_, boost::asio::buffer(out_buffer, strlen(out_buffer) + 3));
		memset(out_buffer, '\0', strlen(out_buffer) + 3);
	}

	void receive(char * in_buffer, tcp::socket& socket_)
	{
		if (in_buffer == nullptr)
			return;

		int n = boost::asio::read(socket_, boost::asio::buffer(in_buffer, 4011), boost::asio::transfer_at_least(1));
	}



	void receive_file(char * in_buffer, tcp::socket& socket_, std::fstream&& fs)
	{
		if (in_buffer == nullptr || !fs)
			return;
		int n = boost::asio::read(socket_, boost::asio::buffer(in_buffer, 4011), boost::asio::transfer_at_least(1));
		fs.write(in_buffer + data_start, n - data_start - 3);
		char tmp_buf[11] = "00050000\r\n";
		boost::asio::write(socket_, boost::asio::buffer(tmp_buf, 11));
		if (n >= 4011)
			receive_file(in_buffer, socket_, std::move(fs));
		socket_not_clear = true;
	}

	void f_login(tcp::socket& socket_, char* buf)
	{
		std::cout << "Please input your user:" << std::endl;
		char user[user_maxlen];
		std::cin >> user;
		std::cout << "Please input your password:" << std::endl;
		char password[pwd_maxlen];
		std::cin >> password;

		int user_len = strlen(user);
		int password_len = strlen(password);
		strcpy(buf, "0000");

		char tmp[5];
		data_int_to_char(user_len + password_len, tmp);
		strcpy(buf + length_start, tmp);

		strcpy(buf + data_start, user);
		strcpy(buf + data_start + user_len, password);
		add_buffer_over(buf, data_start + user_len + password_len);

		send(buf, socket_);
		receive(buf, socket_);
		std::cout << buf << std::endl;
	}

	void f_query(tcp::socket& socket_, char* buf)
	{
		strcpy(buf, "0001");
		strcpy(buf + length_start, "0000");
		add_buffer_over(buf, data_start);

		send(buf, socket_);
		receive(buf, socket_);
		std::cout << buf << std::endl;
	}

	void f_download(tcp::socket& socket_, char* buf)
	{
		std::cout << "Please input the file you want to download:" << std::endl;
		char filename[30];
		std::cin >> filename;

		int name_len = strlen(filename);
		strcpy(buf, "0002");
		char tmp[5];
		data_int_to_char(name_len, tmp);
		strcpy(buf + length_start, tmp);
		strcpy(buf + data_start, filename);
		add_buffer_over(buf, data_start + name_len);

		send(buf, socket_);


		int n = boost::asio::read(socket_, boost::asio::buffer(buf, 4011), boost::asio::transfer_at_least(1));
		my_ftp::my_ftp_protocol ptl;

		ptl.decode_header(buf);
		if ((static_cast<my_ftp::Request_t>(atoi(ptl.type))) == my_ftp::Request_t::nofil)
		{
			std::cout << (buf + data_start) << endl;
			return;
		}


		std::fstream fs(filename, std::ios_base::out | std::ios_base::binary);
		fs.clear();
		fs.seekg(std::ios_base::beg);
		fs.write(buf + data_start, n - data_start - 3);
		char tmp_buf[11] = "00050000\r\n";
		boost::asio::write(socket_, boost::asio::buffer(tmp_buf, 11));
		if (n >= 4011)
			receive_file(buf, socket_, std::move(fs));
		fs.close();

		

	}

	void f_quit(tcp::socket& socket_, char* buf)
	{
		strcpy(buf, "0003");
		strcpy(buf + length_start, "0000");
		add_buffer_over(buf, data_start);
		send(buf, socket_);
		receive(buf, socket_);
		std::cout << buf << std::endl;
	}



}

int main(int argc, char* argv[])
{
	using boost::asio::ip::tcp;
	using namespace std;
	using namespace my_ftp;

	try
	{

		boost::asio::io_service io_service;

		//		getchar();



		tcp::resolver resolver(io_service);
		tcp::resolver::query query(tcp::v4(), "127.0.0.1", "10001");
		tcp::resolver::iterator iterator = resolver.resolve(query);

		tcp::socket socket_(io_service);
		boost::asio::connect(socket_, iterator);



		//char request[max_length];
		//
		//size_t request_length;

		//char reply[max_length];
		//size_t reply_length;

		
		char buf[buf_maxlen];

		while (true)
		{
			char choice[10];
			std::cout << "Please input your choice :\n";
			std::cout << "lg: (login)\n" << "qe: (query)\n" << "dl: (download)\n" << "qt: (disconnect)" << std::endl;
			std::cin >> choice;

			if (strlen(choice) != 2)
			{
				std::cout << "input is wrong" << std::endl;
				continue;
			}

			bool command_effect = false;

			int i;
			for (i = 0; i < command_count; ++i)
			{
				if (!strcmp(commands[i], choice))
				{
					command_effect = true;
					break;
				}
			}

			if (!command_effect)
			{
				std::cout << "input is wrong" << std::endl;
				continue;
			}

			
			//不加这句，下载大文件后write数据直接读取socket中大文件的最后一次发送
			if (socket_not_clear)
			{
				char buffer[4096];
				boost::asio::read(socket_, boost::asio::buffer(buffer, 4096), boost::asio::transfer_at_least(1));
				socket_not_clear = false;
			}


			switch (i)
			{
			case 0:
				f_login(socket_, buf);
				break;
			case 1:
				f_query(socket_, buf);
				break;
			case 2:
				f_download(socket_, buf);
				break;
			case 3:
				f_quit(socket_, buf);
				exit(0);
				break;
			default:
				break;
			}
			
		}

	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
