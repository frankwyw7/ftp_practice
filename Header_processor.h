#pragma once

#include "Request.h"
#include <memory>
#include <string>
#include <boost/filesystem.hpp>
#include <functional>

namespace my_ftp
{
	class Session;
	class Header_processor
	{
	public:
		Header_processor(boost::filesystem::path& path);
		~Header_processor();
		Request_t parser_header(char* buffer_);

		// "\r\n"Ω·Œ≤
		int procees_request(char* ibuffer, std::shared_ptr<Session> session, char* obuffer);
	private:

		int cmppwd(char* ibuffer, char* obuffer);
		int query_file(char* ibuffer, char* obuffer);
		int download(char* ibuffer, std::shared_ptr<Session> session);

		boost::filesystem::path& path_;

		my_ftp_protocol request;
	};
}