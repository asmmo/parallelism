#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <atomic>
#include <string>
using namespace std::chrono_literals;

class Logger
{
    std::mutex queueMutex;
    std::condition_variable condVar;
    std::queue<std::string> messagesQueue;
    std::thread loggingThread;//background process launcher
    std::atomic_bool exit = false;//safety condition
    void processEntries()//the background process
    {
        // Open log file.
        std::ofstream logFile("log.txt");
        if (logFile.fail()) {
            std::cerr << "Failed to open logfile." << std::endl;
            return;
        }

        // Start processing loop. It process only one message for each iteration
        // to gain more performance
        while (!exit) {
            std::unique_lock lock(queueMutex);

            // Wait for a notification and don't wakeup unless the queue isn't empty.
            condVar.wait(lock, [this]{return !messagesQueue.empty();});

            //log to the file
            logFile << messagesQueue.front() << std::endl;
            messagesQueue.pop();
        }

        //At the end, if the queue has some messages. here you don't need mutexes
        // because you have reached the destructor i.e you won't enqueue any messages anymore
        while(exit && !messagesQueue.empty()){

            //log to the file
            logFile << messagesQueue.front() << std::endl;
            messagesQueue.pop();
        }

    }

public:
    Logger()
    {
        //the default ctor launches the background process task
        loggingThread = std::thread{ &Logger::processEntries, this };
    }
    Logger(const Logger& src) = delete;
    Logger& operator=(const Logger& rhs) = delete;

    //logs the messages to the queue
    void log(std::string_view entry)
    {
        std::unique_lock lock(queueMutex);
        messagesQueue.push(std::string(entry));
        condVar.notify_all();
    }


    ~Logger()
    {
        exit = true;
        loggingThread.join();
    }

};


int main(){

    Logger lg;
    for(int i = 1;i < 10000;i++){
        lg.log("This is the message number " + std::to_string(i));
    }

    std::this_thread::sleep_for(10ms);

    for(int i = 10000;i < 20001;i++){
        lg.log("This is the message number " + std::to_string(i));
    }

}
