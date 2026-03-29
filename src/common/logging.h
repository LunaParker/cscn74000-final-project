#ifndef CSCN74000_PROJECT_LOGGING_H
#define CSCN74000_PROJECT_LOGGING_H

#include <string>
#include <fstream>

class Logger {
public:
    Logger();
    ~Logger();

    //Deleted copy constructor and assignment.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Logs a communication event.
     * @param sender The ID or name of the sender.
     * @param direction "INCOMING" or "OUTGOING".
     * @param message The content or packet type description.
     */
    void logEvent(const std::string& sender, const std::string& direction, const std::string& message);

    private:
        std::ofstream logFile;
        void openDailyFile(); // Creates file using the current date
};
#endif
