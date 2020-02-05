#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <thread>
#include <chrono>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/function.hpp>
#include "threadSafeVector.hpp"
#include "session.hpp"
#include "macro_definition.h"

using boost::asio::ip::tcp;

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

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Too few parametrs\n";
        return 1;
    }
    try
    {
        // Start timer
        TimeClicker clk;
        clk.start();
        
        // Init main processing modules
        boost::shared_ptr<boost::asio::io_service> service(new boost::asio::io_service);
        boost::shared_ptr<MessageProcessor> msg_proc_ptr(new MessageProcessor);
        boost::shared_ptr<MessagePool> msg_pool(new MessagePool(COUNTER_FINISH));
        
        // Create all sockets
        
        tcp::endpoint ep( boost::asio::ip::address::from_string(argv[1]), std::stoi(argv[2]));
        std::vector<boost::shared_ptr<Session>> ses_vec;
        for (int i = 0; i < SOCKET_COUNT; i++)
        {
            ses_vec.emplace_back(new Session(service, ep));
            ses_vec.back() -> setMsgPool(msg_pool);
            ses_vec.back() -> addMesgProcessor(msg_proc_ptr);
        }
        
        // Init connections, connect to server
        for (auto& i : ses_vec)
        {
            i->start();
        }
        
        // Create all thrads
        boost::thread_group threads;
        for ( int i = 0; i < THREAD_COUNT; ++i)
        {
            threads.create_thread(boost::bind(&boost::asio::io_service::run, service));
        }
        threads.join_all();
        
        std::cout << "\nMED IS " << msg_proc_ptr -> countMed() << "\n";
        // Print results
        std::cout << "\nVector size " << msg_proc_ptr -> getData().size() << "\n";
        
        
        // Stop timer  and print total duration
        clk.stop();
        std::cout << "\n================= TOTAL DURATION IS " << clk.duration().count() << " ======================\n";
        }
    catch( ... )
    {
        std::cerr << "Some error occured\n";
        return 1;
    }
    return 0;
    
}
