#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <map>

using namespace std;

// Include command parser
#include "command.h"

// Simple implementation with linear search
const int MAX_USERS = 10000;
const int MAX_TRAINS = 1000;
const int MAX_ORDERS = 100000;

struct User {
    char username[25];
    char password[35];
    char name[16];
    char mailAddr[35];
    int privilege;
    bool loggedIn;
    bool exists;

    User() : privilege(0), loggedIn(false), exists(false) {
        username[0] = '\0';
    }
};

struct Train {
    char trainID[25];
    int stationNum;
    char stations[100][31];
    int seatNum;
    int prices[99];
    int startHour, startMinute;
    int travelTimes[99];
    int stopoverTimes[98];
    int saleDateStartMonth, saleDateStartDay;
    int saleDateEndMonth, saleDateEndDay;
    char type;
    bool released;
    bool exists;

    Train() : stationNum(0), seatNum(0), released(false), exists(false) {
        trainID[0] = '\0';
    }
};

User users[MAX_USERS];
Train trains[MAX_TRAINS];
int userCount = 0;
int trainCount = 0;

// Simple linear search functions
int findUser(const char* username) {
    for (int i = 0; i < userCount; i++) {
        if (users[i].exists && strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}

int findTrain(const char* trainID) {
    for (int i = 0; i < trainCount; i++) {
        if (trains[i].exists && strcmp(trains[i].trainID, trainID) == 0) {
            return i;
        }
    }
    return -1;
}

void parseTime(const char* timeStr, int& hour, int& minute) {
    sscanf(timeStr, "%d:%d", &hour, &minute);
}

void parseDate(const char* dateStr, int& month, int& day) {
    sscanf(dateStr, "%d-%d", &month, &day);
}

// Helper for parsing pipe-separated values
void parsePipeValues(const char* str, int values[], int count) {
    char buffer[5000];
    strcpy(buffer, str);
    char* token = strtok(buffer, "|");
    int idx = 0;
    while (token && idx < count) {
        if (strcmp(token, "_") == 0) {
            values[idx] = 0;
        } else {
            values[idx] = atoi(token);
        }
        token = strtok(nullptr, "|");
        idx++;
    }
}

void parsePipeStrings(const char* str, char strings[][31], int count) {
    char buffer[5000];
    strcpy(buffer, str);
    char* token = strtok(buffer, "|");
    int idx = 0;
    while (token && idx < count) {
        strcpy(strings[idx], token);
        token = strtok(nullptr, "|");
        idx++;
    }
}

// Command implementations
int add_user(const char* curUser, const char* username, const char* password,
             const char* name, const char* mailAddr, int privilege) {
    if (findUser(username) != -1) {
        return -1;
    }

    if (userCount == 0) {
        User& user = users[userCount++];
        strcpy(user.username, username);
        strcpy(user.password, password);
        strcpy(user.name, name);
        strcpy(user.mailAddr, mailAddr);
        user.privilege = 10;
        user.exists = true;
        return 0;
    }

    int curIdx = findUser(curUser);
    if (curIdx == -1 || !users[curIdx].loggedIn) {
        return -1;
    }

    if (privilege >= users[curIdx].privilege) {
        return -1;
    }

    User& user = users[userCount++];
    strcpy(user.username, username);
    strcpy(user.password, password);
    strcpy(user.name, name);
    strcpy(user.mailAddr, mailAddr);
    user.privilege = privilege;
    user.exists = true;
    return 0;
}

int login(const char* username, const char* password) {
    int idx = findUser(username);
    if (idx == -1) {
        return -1;
    }

    User& user = users[idx];
    if (strcmp(user.password, password) != 0) {
        return -1;
    }

    if (user.loggedIn) {
        return -1;
    }

    user.loggedIn = true;
    return 0;
}

int logout(const char* username) {
    int idx = findUser(username);
    if (idx == -1 || !users[idx].loggedIn) {
        return -1;
    }

    users[idx].loggedIn = false;
    return 0;
}

int query_profile(const char* curUser, const char* username) {
    int curIdx = findUser(curUser);
    int targetIdx = findUser(username);

    if (curIdx == -1 || targetIdx == -1) {
        return -1;
    }

    User& cur = users[curIdx];
    User& target = users[targetIdx];

    if (!cur.loggedIn) {
        return -1;
    }

    if (cur.privilege <= target.privilege && strcmp(cur.username, target.username) != 0) {
        return -1;
    }

    printf("%s %s %s %d\n", target.username, target.name, target.mailAddr, target.privilege);
    return 0;
}

int modify_profile(const char* curUser, const char* username, const char* password,
                   const char* name, const char* mailAddr, int privilege) {
    int curIdx = findUser(curUser);
    int targetIdx = findUser(username);

    if (curIdx == -1 || targetIdx == -1) {
        return -1;
    }

    User& cur = users[curIdx];
    User& target = users[targetIdx];

    if (!cur.loggedIn) {
        return -1;
    }

    if (cur.privilege <= target.privilege && strcmp(cur.username, target.username) != 0) {
        return -1;
    }

    if (privilege != -1 && privilege >= cur.privilege) {
        return -1;
    }

    if (password != nullptr && strlen(password) > 0) {
        strcpy(target.password, password);
    }
    if (name != nullptr && strlen(name) > 0) {
        strcpy(target.name, name);
    }
    if (mailAddr != nullptr && strlen(mailAddr) > 0) {
        strcpy(target.mailAddr, mailAddr);
    }
    if (privilege != -1) {
        target.privilege = privilege;
    }

    printf("%s %s %s %d\n", target.username, target.name, target.mailAddr, target.privilege);
    return 0;
}

int add_train(const char* trainID, int stationNum, int seatNum, const char* stationsStr,
              const char* pricesStr, const char* startTime, const char* travelTimesStr,
              const char* stopoverTimesStr, const char* saleDateStr, char type) {
    if (findTrain(trainID) != -1) {
        return -1;
    }

    Train& train = trains[trainCount++];
    strcpy(train.trainID, trainID);
    train.stationNum = stationNum;
    train.seatNum = seatNum;
    train.type = type;
    train.released = false;
    train.exists = true;

    // Parse stations
    parsePipeStrings(stationsStr, train.stations, stationNum);

    // Parse prices
    parsePipeValues(pricesStr, train.prices, stationNum - 1);

    // Parse start time
    parseTime(startTime, train.startHour, train.startMinute);

    // Parse travel times
    parsePipeValues(travelTimesStr, train.travelTimes, stationNum - 1);

    // Parse stopover times
    if (stationNum > 2) {
        parsePipeValues(stopoverTimesStr, train.stopoverTimes, stationNum - 2);
    }

    // Parse sale date
    char buffer[100];
    strcpy(buffer, saleDateStr);
    char* token = strtok(buffer, "|");
    parseDate(token, train.saleDateStartMonth, train.saleDateStartDay);
    token = strtok(nullptr, "|");
    parseDate(token, train.saleDateEndMonth, train.saleDateEndDay);

    return 0;
}

int release_train(const char* trainID) {
    int idx = findTrain(trainID);
    if (idx == -1) {
        return -1;
    }

    trains[idx].released = true;
    return 0;
}

int delete_train(const char* trainID) {
    int idx = findTrain(trainID);
    if (idx == -1) {
        return -1;
    }

    if (trains[idx].released) {
        return -1;
    }

    trains[idx].exists = false;
    return 0;
}

// Helper function to add minutes to time
void addMinutes(int& day, int& hour, int& minute, int minutes) {
    minute += minutes;
    hour += minute / 60;
    minute %= 60;
    day += hour / 24;
    hour %= 24;
}

int query_train(const char* trainID, const char* dateStr) {
    int idx = findTrain(trainID);
    if (idx == -1) {
        return -1;
    }

    Train& train = trains[idx];
    int queryMonth, queryDay;
    parseDate(dateStr, queryMonth, queryDay);

    // Simple date check (not fully accurate but works for basic cases)
    if (queryMonth < train.saleDateStartMonth || queryMonth > train.saleDateEndMonth) {
        return -1;
    }
    if (queryMonth == train.saleDateStartMonth && queryDay < train.saleDateStartDay) {
        return -1;
    }
    if (queryMonth == train.saleDateEndMonth && queryDay > train.saleDateEndDay) {
        return -1;
    }

    printf("%s %c\n", train.trainID, train.type);

    // Calculate times
    int currentDay = 1; // Simplified day counter
    int currentHour = train.startHour;
    int currentMinute = train.startMinute;
    int cumulativePrice = 0;

    for (int i = 0; i < train.stationNum; i++) {
        char arriveTime[20], leaveTime[20];

        if (i == 0) {
            strcpy(arriveTime, "xx-xx xx:xx");
        } else {
            sprintf(arriveTime, "%02d-%02d %02d:%02d", queryMonth, queryDay, currentHour, currentMinute);
        }

        // Add stopover time (except for first and last station)
        if (i > 0 && i < train.stationNum - 1) {
            addMinutes(currentDay, currentHour, currentMinute, train.stopoverTimes[i-1]);
        }

        if (i == train.stationNum - 1) {
            strcpy(leaveTime, "xx-xx xx:xx");
        } else {
            sprintf(leaveTime, "%02d-%02d %02d:%02d", queryMonth, queryDay, currentHour, currentMinute);
        }

        char seatInfo[10];
        if (i == train.stationNum - 1) {
            strcpy(seatInfo, "x");
        } else {
            sprintf(seatInfo, "%d", train.seatNum);
        }

        printf("%s %s -> %s %d %s\n", train.stations[i], arriveTime, leaveTime,
               cumulativePrice, seatInfo);

        // Add travel time to next station (except for last station)
        if (i < train.stationNum - 1) {
            addMinutes(currentDay, currentHour, currentMinute, train.travelTimes[i]);
            cumulativePrice += train.prices[i];
        }
    }

    return 0;
}

int query_ticket(const char* fromStation, const char* toStation,
                 const char* dateStr, const char* sortBy) {
    // Simple implementation: check all trains
    int resultCount = 0;

    for (int i = 0; i < trainCount; i++) {
        if (!trains[i].exists || !trains[i].released) continue;

        Train& train = trains[i];

        // Find fromStation and toStation indices
        int fromIdx = -1, toIdx = -1;
        for (int j = 0; j < train.stationNum; j++) {
            if (strcmp(train.stations[j], fromStation) == 0) {
                fromIdx = j;
            }
            if (fromIdx != -1 && j > fromIdx && strcmp(train.stations[j], toStation) == 0) {
                toIdx = j;
                break;
            }
        }

        if (fromIdx != -1 && toIdx != -1) {
            // Check date (simplified)
            int queryMonth, queryDay;
            parseDate(dateStr, queryMonth, queryDay);

            if (queryMonth >= train.saleDateStartMonth && queryMonth <= train.saleDateEndMonth) {
                resultCount++;
            }
        }
    }

    printf("%d\n", resultCount);

    // Output results (simplified)
    for (int i = 0; i < trainCount && resultCount > 0; i++) {
        if (!trains[i].exists || !trains[i].released) continue;

        Train& train = trains[i];

        int fromIdx = -1, toIdx = -1;
        for (int j = 0; j < train.stationNum; j++) {
            if (strcmp(train.stations[j], fromStation) == 0) {
                fromIdx = j;
            }
            if (fromIdx != -1 && j > fromIdx && strcmp(train.stations[j], toStation) == 0) {
                toIdx = j;
                break;
            }
        }

        if (fromIdx != -1 && toIdx != -1) {
            int queryMonth, queryDay;
            parseDate(dateStr, queryMonth, queryDay);

            if (queryMonth >= train.saleDateStartMonth && queryMonth <= train.saleDateEndMonth) {
                // Calculate price
                int price = 0;
                for (int k = fromIdx; k < toIdx; k++) {
                    price += train.prices[k];
                }

                // Simplified time calculation
                printf("%s %s %02d-%02d %02d:%02d -> %s %02d-%02d %02d:%02d %d %d\n",
                       train.trainID,
                       train.stations[fromIdx],
                       queryMonth, queryDay, train.startHour, train.startMinute,
                       train.stations[toIdx],
                       queryMonth, queryDay, train.startHour, train.startMinute,
                       price, train.seatNum);

                resultCount--;
            }
        }
    }

    return 0;
}

int clean() {
    userCount = 0;
    trainCount = 0;
    return 0;
}

int exit_program() {
    printf("bye\n");
    return 0;
}

// Command parser
CommandParser parser;

void processCommand(const string& line) {
    if (line.empty()) return;

    parser.parse(line);

    if (parser.command == "add_user") {
        const char* curUser = parser.get("c").c_str();
        const char* username = parser.get("u").c_str();
        const char* password = parser.get("p").c_str();
        const char* name = parser.get("n").c_str();
        const char* mailAddr = parser.get("m").c_str();
        int privilege = parser.getInt("g");

        int result = add_user(curUser, username, password, name, mailAddr, privilege);
        printf("%d\n", result);

    } else if (parser.command == "login") {
        const char* username = parser.get("u").c_str();
        const char* password = parser.get("p").c_str();

        int result = login(username, password);
        printf("%d\n", result);

    } else if (parser.command == "logout") {
        const char* username = parser.get("u").c_str();

        int result = logout(username);
        printf("%d\n", result);

    } else if (parser.command == "query_profile") {
        const char* curUser = parser.get("c").c_str();
        const char* username = parser.get("u").c_str();

        int result = query_profile(curUser, username);
        if (result == -1) {
            printf("-1\n");
        }

    } else if (parser.command == "modify_profile") {
        const char* curUser = parser.get("c").c_str();
        const char* username = parser.get("u").c_str();
        const char* password = parser.has("p") ? parser.get("p").c_str() : nullptr;
        const char* name = parser.has("n") ? parser.get("n").c_str() : nullptr;
        const char* mailAddr = parser.has("m") ? parser.get("m").c_str() : nullptr;
        int privilege = parser.has("g") ? parser.getInt("g") : -1;

        int result = modify_profile(curUser, username, password, name, mailAddr, privilege);
        if (result == -1) {
            printf("-1\n");
        }

    } else if (parser.command == "add_train") {
        const char* trainID = parser.get("i").c_str();
        int stationNum = parser.getInt("n");
        int seatNum = parser.getInt("m");
        const char* stations = parser.get("s").c_str();
        const char* prices = parser.get("p").c_str();
        const char* startTime = parser.get("x").c_str();
        const char* travelTimes = parser.get("t").c_str();
        const char* stopoverTimes = parser.get("o").c_str();
        const char* saleDate = parser.get("d").c_str();
        char type = parser.get("y")[0];

        int result = add_train(trainID, stationNum, seatNum, stations, prices,
                              startTime, travelTimes, stopoverTimes, saleDate, type);
        printf("%d\n", result);

    } else if (parser.command == "release_train") {
        const char* trainID = parser.get("i").c_str();

        int result = release_train(trainID);
        printf("%d\n", result);

    } else if (parser.command == "delete_train") {
        const char* trainID = parser.get("i").c_str();

        int result = delete_train(trainID);
        printf("%d\n", result);

    } else if (parser.command == "query_train") {
        const char* trainID = parser.get("i").c_str();
        const char* date = parser.get("d").c_str();

        int result = query_train(trainID, date);
        if (result == -1) {
            printf("-1\n");
        }

    } else if (parser.command == "query_ticket") {
        const char* fromStation = parser.get("s").c_str();
        const char* toStation = parser.get("t").c_str();
        const char* date = parser.get("d").c_str();
        const char* sortBy = parser.get("p", "time").c_str();

        int result = query_ticket(fromStation, toStation, date, sortBy);
        if (result == -1) {
            printf("-1\n");
        }

    } else if (parser.command == "clean") {
        int result = clean();
        printf("%d\n", result);

    } else if (parser.command == "exit") {
        exit_program();
        exit(0);

    } else {
        printf("-1\n");
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string line;
    while (getline(cin, line)) {
        processCommand(line);
    }

    return 0;
}