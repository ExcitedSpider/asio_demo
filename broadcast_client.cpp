#include<iostream>
using std::cout;
using std::endl;

#define BOOST_ASIO_DISABLE_STD_CHRONO
#define BOOST_THREAD_VERSION 5
#define MAX_SOCKET_AMOUNT 4
#define BUFFER_SIZE 1024
#include<boost/thread.hpp>
#include<boost/asio.hpp>
#include<boost/smart_ptr.hpp>
#include<boost/bind.hpp>
#include<iostream>
#include<boost/asio/steady_timer.hpp>
#include<boost/asio/system_timer.hpp>
#include<boost/asio/high_resolution_timer.hpp>
#include<string>
#include<boost/function.hpp>
#include<vector>
#include<boost/atomic.hpp>
using namespace boost::asio;
using boost::system::error_code;
using ip::tcp;
using std::cout; using std::endl;

class BroadcastClient {
	typedef boost::shared_ptr<ip::tcp::socket> sockptr;
	typedef boost::shared_array<char> buffers;
private:
	io_service& io;
	buffers b;
	sockptr sock;

	void do_read() {
		cout << b.get() << endl;
		sock->async_read_some(buffer(b.get(), 1024 * sizeof(char)), boost::bind(&BroadcastClient::read_handler, this, _1));
	}

	void read_handler(error_code ec)
	{
		if (ec) return;
		do_read();
	}

	void connect_handler(error_code ec)
	{
		if (ec) return;
		sock->async_read_some(buffer(b.get(), 1024 * sizeof(char)), boost::bind(&BroadcastClient::read_handler, this, _1));
	}

public:
	BroadcastClient(io_service& io_) :io(io_), b(new char[1024]), sock(new ip::tcp::socket(io))
	{
		memset(b.get(), '\0', 1024);
	};
	void start()
	{
		ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 666);
		sock->async_connect(ep, boost::bind(&BroadcastClient::connect_handler, this, _1 ));
		io.run();
	};
};

int main() {
	io_service io;
	BroadcastClient client(io);
	client.start();
	io.run();
}