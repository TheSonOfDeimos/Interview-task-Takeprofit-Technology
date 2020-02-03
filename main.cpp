#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <chrono>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include "threadSafeVector.hpp"

//==========================================
#define COUNTER_FINSH 2018
#define COUNTER_START 1
#define THREAD_COUNT 7
#define SOCKET_COUNT 100

//=========================================

using boost::asio::ip::tcp;
boost::asio::io_service service_;
SafeVector s_vec_;
std::atomic<int> counter_(COUNTER_START);

class TimeClicker
{
public:
    void start()
    {
        time_start_ = std::chrono::system_clock::now();
    }
    
    void stop()
    {
        time_finish_ = std::chrono::system_clock::now();
    }
    
    std::chrono::duration<double> duration()
    {
        return time_finish_ - time_start_;
    }
    
private:
    std::chrono::time_point<std::chrono::system_clock> time_start_;
    std::chrono::time_point<std::chrono::system_clock> time_finish_;
};

class Session
{
public:
  Session(boost::asio::io_service& io_service, tcp::endpoint& ep)
    : socket_(io_service),
    ep_(ep)
  {
  }

  tcp::socket& socket()
  {
      return socket_;
  }

  void start()
  {
      socket_.async_connect(ep_, boost::bind(&Session::connect_handler, this,
      boost::asio::placeholders::error));
  }

private:
    void processMsg(std::string str)
    {
        str.erase(std::remove_if(str.begin(), str.end(), ::ispunct), str.end());
        s_vec_.push_back(std::stoi(str));
        
    }
    
    void connect_handler(const boost::system::error_code& error)
    {
        if (!error) {
        
            int cnt = std::atomic_fetch_add(&counter_, 1);
            if (cnt <= COUNTER_FINSH) {
                std::cout << "Counter: " << cnt << "thread id" << boost::this_thread::get_id() <<  "\n";
                socket_.async_write_some(boost::asio::buffer(std::to_string(cnt) + "\n"),
                                         boost::bind(&Session::handle_write, this, boost::asio::placeholders::error));
            }
        } else {
            std::cout << "\nERROR CONNECT\n";
            delete this;
        }
    }
    
  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error) {
        std::string str((std::istreambuf_iterator<char>(&buff_)), std::istreambuf_iterator<char>());
        //std::cout << " Result: " << str << "\n";
        service_.post(boost::bind(&Session::processMsg, this, str));
        
        int cnt = std::atomic_fetch_add(&counter_, 1);
        
        if (cnt <= COUNTER_FINSH) {
            std::cout << "Counter: " << cnt << "thread id " << boost::this_thread::get_id() << "\n";
            socket_.async_write_some(boost::asio::buffer(std::to_string(cnt) + "\n"),
                                     boost::bind(&Session::handle_write, this, boost::asio::placeholders::error));
        }
    } else {
        std::cout << "\nERROR READ\n";
        delete this;
    }
  }

  void handle_write(const boost::system::error_code& error)
  {
    if (!error) {
        boost::asio::async_read_until(socket(), buff_, '\n',
                                      boost::bind(&Session::handle_read, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    } else {
        std::cout << "\nERROR WRITE\n";
        delete this;
    }
  }

    tcp::socket socket_;
    boost::asio::streambuf buff_;
    tcp::endpoint& ep_;
};

void run_service()
{
    service_.run();
}

int main(int argc, char* argv[])
{

    // Start timer
    TimeClicker clk;
    clk.start();
    
    // Create all sockets
    tcp::endpoint ep( boost::asio::ip::address::from_string("195.133.144.219"), 2012);
    std::vector<boost::shared_ptr<Session>> ses_vec;
    for (int i = 0; i < SOCKET_COUNT; i++)
    {
        ses_vec.emplace_back(new Session(service_, ep));
    }
    
    // Init connections? connect to server
    for (auto& i : ses_vec)
    {
        i->start();
    }
    
    // Create all thrads
    boost::thread_group threads;
    for ( int i = 0; i < THREAD_COUNT; ++i)
    {
        threads.create_thread(boost::bind(&boost::asio::io_service::run, &service_));
    }
    threads.join_all();
    
    // Print results
    std::cout << "\nVector size " << s_vec_.toVector().size() << "\n";
    std::copy(s_vec_.begin(), s_vec_.end(), std::ostream_iterator<int>(std::cout, "\n"));
    
    // Stop timer  and print total duration
    clk.stop();
    std::cout << "\n\n\n================= TOTAL DURATION IS " << clk.duration().count() << " ======================\n\n\n";
    
    return 0;
    
}
