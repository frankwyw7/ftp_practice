#include "Server_manager.h"
#include "Session.h"
#include <algorithm>

namespace my_ftp
{
	Server_manager::Server_manager()
	{

	}

	Server_manager::~Server_manager()
	{

	}

	void Server_manager::add_session(std::shared_ptr<Session> session)
	{
		sessions.insert(session);
		session->start();
	}

	void Server_manager::delete_session(std::shared_ptr<Session> session)
	{
		sessions.erase(session);
		session->stop();
	}

	void Server_manager::delete_all()
	{
		std::for_each(sessions.begin(), sessions.end(), [](std::shared_ptr<Session> session) {session->stop(); });
		sessions.clear();
	}
}