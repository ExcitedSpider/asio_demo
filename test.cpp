#include <iostream>  
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace std;


void Print(const boost::system::error_code &ec,
	boost::asio::deadline_timer* pt,
	int * pcount)
{
	int a[1000];
	cout << "count = " << *pcount << endl;
	cout << boost::this_thread::get_id() << endl;
	(*pcount)++;

	pt->expires_at(pt->expires_at() + boost::posix_time::seconds(1));

	pt->async_wait(boost::bind(Print, boost::asio::placeholders::error, pt, pcount));

}
/*
int main()
{
	cout << boost::this_thread::get_id() << endl;
	boost::asio::io_service io;
	boost::asio::deadline_timer t(io, boost::posix_time::seconds(1));
	int count = 0;
	t.async_wait(boost::bind(Print, boost::asio::placeholders::error, &t, &count));
	cout << "to run" << endl;
	io.run();
	cout << "Final count is " << count << "\n";
	cout << "exit" << endl;
	return 0;
}
*/