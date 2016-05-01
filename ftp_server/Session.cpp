#include "Session.h"
#include "Header_processor.h"
#include "Server_manager.h"
#include "../common/help_function.h"
#include <fstream>
#include <iostream>
#include <boost/bind.hpp>


namespace my_ftp
{
	using socket = ip::tcp::socket;

	static const int DATA_LEN = 4000;
	static const int HEADER_LEN = 8;
	static const int OVER_LEN = 3;//\r\n

	Session::Session(io_service& io_service_, std::shared_ptr<Server_manager> server_manager__)
		:socket_(io_service_), server_manager_(server_manager__), 
		path_(boost::filesystem::current_path() /= "FTP"),
		header_processor_(std::make_unique<Header_processor>(path_)),
		deadline_timer_(io_service_),
		go_on(true),
		mutex_()
	{ 
		
		
	}

	Session::~Session() 
	{
		pruntime("~Session des");
	}

	void Session::stop()
	{
		char over[] = "quit now";
		write(socket_, buffer(over));

		{
			std::lock_guard<std::mutex> lock(mutex_);
			go_on = false;
		}
		socket_.close();
	}

	socket& Session::get_socket()
	{
		return socket_;
	}

	void Session::start()
	{
		try
		{
			deadline_timer_.expires_from_now(boost::posix_time::seconds(30));
			pruntime("Session start");
			
			async_read(socket_, buffer(input_buffer_), transfer_at_least(6),
				boost::bind(&Session::handle_read, shared_from_this(), boost::asio::placeholders::error));
			deadline_timer_.async_wait(boost::bind(&Session::check_deadline, shared_from_this()));
		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}	

	}

	void Session::handle_read(const boost::system::error_code& error)
	{		
		
		pruntime("Session handle_read");
		try
		{
			deadline_timer_.expires_from_now(boost::posix_time::seconds(30));
			if (!error && go_on)
			{
				int result = 0;
				result = header_processor_->procees_request(input_buffer_, shared_from_this(), output_buffer_);


				if (result == -1)
				{
					this->get_manager()->delete_session(shared_from_this());
					return;
				}
				//下载文件
				if (result == 1)
				{
					fs.read(output_buffer_ + HEADER_LEN, DATA_LEN);
					int n = fs.gcount() ;
					char n_char[5];
					data_int_to_char(n, n_char);
					strcpy(output_buffer_, "0006");
					for (int i = 0; i < 4; ++i)
						output_buffer_[i + 4] = n_char[i];
					if (n < DATA_LEN)
					{
						add_buffer_over(output_buffer_, n + HEADER_LEN);
						async_write(socket_, buffer(output_buffer_, HEADER_LEN + n + OVER_LEN), transfer_at_least(1),
							boost::bind(&Session::handle_file_write, shared_from_this(),
								boost::asio::placeholders::error));
					}
					else
					{
						add_buffer_over(output_buffer_, n + HEADER_LEN);
						async_write(socket_, buffer(output_buffer_, HEADER_LEN + DATA_LEN + OVER_LEN), transfer_at_least(1),
							boost::bind(&Session::handle_go_write, shared_from_this(),
								boost::asio::placeholders::error));
					}
				}
				//直接发送输出buffer
				else if (result == 0)
				{
					add_buffer_over(output_buffer_, strlen(output_buffer_));
					async_write(socket_, buffer(output_buffer_, strlen(output_buffer_)+ OVER_LEN), transfer_at_least(1),
						boost::bind(&Session::handle_write, shared_from_this(), boost::asio::placeholders::error));
				}
			}
			else
			{
				perr("Session handle read");
				std::cerr << error << std::endl;
			}
		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}
	}

	void Session::handle_write(const boost::system::error_code& error)
	{
		pruntime("Session handle_write");
		try
		{
			deadline_timer_.expires_from_now(boost::posix_time::seconds(30));
			if (fs)
			{
				fs.close();
				fs.clear();
			}
			if (!error && go_on)
			{
				start();
			}
			else
			{
				perr("Session handle write");
			}
		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}
	}

	void Session::handle_file_write(const boost::system::error_code& error)
	{

		pruntime("Session handle_write");
		if (!receive_trnbk())
		{
			perr("not receive trans back");
			return;
		}
		try
		{
			deadline_timer_.expires_from_now(boost::posix_time::seconds(30));
			if (fs.is_open())
			{
				fs.close();
				fs.clear();
			}			
			if (!error && go_on)
			{
				start();
			}
			else
			{
				perr("Session handle write");
			}
		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}
		
	}

	//再次wirite一定量的数据，如果尚未传输完成，递归（传递fs目前读取的位移）
	void Session::handle_go_write(const boost::system::error_code& error)
	{
		pruntime("Session handle_go_write");
		if (!receive_trnbk())
		{
			perr("not receive trans back");
			return;
		}
		try
		{
			if (!error && go_on)
			{
				deadline_timer_.expires_from_now(boost::posix_time::seconds(30));

				strcpy(output_buffer_, "0006");
				strcpy(output_buffer_+ 4, "0000");
				fs.read(output_buffer_ + HEADER_LEN, DATA_LEN);
				int n = fs.gcount();

				if (n < DATA_LEN)
				{
					add_buffer_over(output_buffer_, n + HEADER_LEN);
					async_write(socket_, buffer(output_buffer_, HEADER_LEN + n + OVER_LEN), transfer_at_least(1),
						boost::bind(&Session::handle_write, shared_from_this(),
							boost::asio::placeholders::error));
				}
				else
				{
					add_buffer_over(output_buffer_, n + HEADER_LEN);
					async_write(socket_, buffer(output_buffer_, HEADER_LEN + n + OVER_LEN), transfer_at_least(1),
						boost::bind(&Session::handle_go_write, shared_from_this(),
							boost::asio::placeholders::error));
				}

			}
			else
			{
				perr("Session handle_go_write");
			}
		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}		
	}	

	std::shared_ptr<Server_manager> Session::get_manager()
	{
		return server_manager_;
	}

	bool Session::set_downld_flie(std::string file)
	{
		fs.open(file, std::ios_base::binary | std::ios_base::in);
		if (fs.is_open())
		{
			fs.clear();
			fs.seekg(std::ios::beg);
			return true;
		}
		return false;
	}

	void Session::check_deadline()
	{		
		if (deadline_timer_.expires_at() <= deadline_timer::traits_type::now())
		{
			{
				std::lock_guard<std::mutex> lock(mutex_);
				go_on = false;
			}
			
			server_manager_->delete_session(shared_from_this());
			return;
		}
		deadline_timer_.async_wait(boost::bind(&Session::check_deadline, shared_from_this()));
	}

	bool Session::receive_trnbk()
	{
		try
		{
			pruntime("receive_trnbk");
			deadline_timer_.expires_from_now(boost::posix_time::seconds(30));
			char tmp_buf[11];
			boost::asio::read(socket_, boost::asio::buffer(tmp_buf, 11));
			my_ftp_protocol ptl;
			ptl.decode(tmp_buf);
			if (static_cast<Request_t>(atoi(ptl.type)) != trnbk)
			{
				perr("valid transfile back");
				return false;
			}
			return true;
		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
			return false;
		}
	}
}