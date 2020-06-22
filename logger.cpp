/*

https://codereview.stackexchange.com/questions/244257/c-multithreading-logger-class/244302#244302
*/


#include <thread>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <string>
#include <ctime>
using namespace std::chrono_literals;

struct Helper{

    static std::ofstream creatLogFile(){
        std::ofstream logFile(getFileName());
        if (logFile.fail()) {
            std::cout << "Failed to open logfile. Did u give a valid "
                         "name and Do u have the permission" << std::endl;
            return std::move(creatLogFile());
        }
        return logFile;
    }

    static std::string getFileName(){
        std::string toBeReturned;
        std::cout << "Do you want to have a custom name for your logger (Y/N):\t";
        char flag;
        while(! (std::cin >> flag)){
            std::cout << "Plz, Enter (Y/N)\t:";
            std::cin.clear();
        }
        if(toupper(flag) == 'Y')
        {
            std::cout << "Please write a name for your logging file:\t";
            while(! (std::cin >> toBeReturned)){
                std::cout << "Plz, Enter a valid name:";
                std::cin.clear();
            }
            toBeReturned += ".txt";
        }
        else {
            time_t currentTime{};
            time(&currentTime);
            tm localCurrentTime{};
            localtime_s(&localCurrentTime, &currentTime);
            char localCurrentTimeStr[30];
            asctime_s(localCurrentTimeStr, &localCurrentTime);
            auto timeOnFileName = std::string{localCurrentTimeStr};
            timeOnFileName.pop_back();
            std::for_each(timeOnFileName.begin(), timeOnFileName.end(),
                          [](auto &el) { if (el == ':') el = '_'; });
            toBeReturned = "log  " + timeOnFileName + ".txt";
        }
        return toBeReturned;
    }

};


class Logger
{
    std::mutex queueMutex;
    std::condition_variable condVar;
    std::queue<std::string> messagesQueue;
    std::thread loggingThread;//background process launcher
    bool exit = false;//safety condition

    void processEntries()//the background process
    {
        std::ofstream logFile = Helper::creatLogFile();


        // Start processing loop. It process only one message for each iteration
        // to gain more performance

        while (true) {//The loop will exit only when exit is true
            std::unique_lock lock(queueMutex);

            // Wait for a notification and don't wakeup unless the queue isn't empty or the exit is true.
            condVar.wait(lock, [this]{return !messagesQueue.empty() || exit;});

            //log to the file if there are messages to be logged
            if(!messagesQueue.empty()) {
                logFile << messagesQueue.front() << "\n";
                messagesQueue.pop();
            }
            
            if(exit) break;//The loop will exit only when exit is true
        }

        //At the end, if the queue has some messages. here you don't need mutexes
        // because you have reached the destructor i.e you won't enqueue any messages
        // or change exit anymore
        while(!messagesQueue.empty()){

            //log to the file
            logFile << messagesQueue.front() << "\n";
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
        std::lock_guard lock(queueMutex);
        messagesQueue.push(std::string(entry));
        condVar.notify_all();
    }


    ~Logger()
    {
        if(loggingThread.joinable()){
            {
                std::lock_guard lg {queueMutex};
                exit = true;
            }
            condVar.notify_all();
            loggingThread.join();
        }
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
