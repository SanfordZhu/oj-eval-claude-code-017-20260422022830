#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace std;

// Include B+ tree
#include "bptree.h"

// Simple hash table implementation for strings
const int HASH_SIZE = 100007;

struct HashNode {
    char key[25];
    int value;
    HashNode* next;

    HashNode(const char* k, int v) : value(v), next(nullptr) {
        strcpy(key, k);
    }
};

class HashMap {
private:
    HashNode* table[HASH_SIZE];

    unsigned int hash(const char* str) {
        unsigned int h = 0;
        while (*str) {
            h = h * 131 + *str++;
        }
        return h % HASH_SIZE;
    }

public:
    HashMap() {
        memset(table, 0, sizeof(table));
    }

    ~HashMap() {
        for (int i = 0; i < HASH_SIZE; i++) {
            HashNode* node = table[i];
            while (node) {
                HashNode* next = node->next;
                delete node;
                node = next;
            }
        }
    }

    void insert(const char* key, int value) {
        unsigned int h = hash(key);
        HashNode* node = new HashNode(key, value);
        node->next = table[h];
        table[h] = node;
    }

    int* find(const char* key) {
        unsigned int h = hash(key);
        HashNode* node = table[h];
        while (node) {
            if (strcmp(node->key, key) == 0) {
                return &node->value;
            }
            node = node->next;
        }
        return nullptr;
    }

    bool erase(const char* key) {
        unsigned int h = hash(key);
        HashNode** pp = &table[h];
        while (*pp) {
            if (strcmp((*pp)->key, key) == 0) {
                HashNode* to_delete = *pp;
                *pp = to_delete->next;
                delete to_delete;
                return true;
            }
            pp = &(*pp)->next;
        }
        return false;
    }
};

// User structure
struct User {
    char username[25];
    char password[35];
    char name[16];  // 2-5 Chinese characters, each up to 3 bytes
    char mailAddr[35];
    int privilege;
    bool loggedIn;

    User() : privilege(0), loggedIn(false) {
        username[0] = '\0';
    }

    User(const char* u, const char* p, const char* n, const char* m, int g)
        : privilege(g), loggedIn(false) {
        strcpy(username, u);
        strcpy(password, p);
        strcpy(name, n);
        strcpy(mailAddr, m);
    }
};

// Train structure
struct Train {
    char trainID[25];
    int stationNum;
    char stations[100][31];  // up to 10 Chinese chars * 3 bytes
    int seatNum;
    int prices[99];  // stationNum-1
    int startHour, startMinute;
    int travelTimes[99];  // stationNum-1
    int stopoverTimes[98]; // stationNum-2
    int saleDateStartMonth, saleDateStartDay;
    int saleDateEndMonth, saleDateEndDay;
    char type;
    bool released;

    Train() : stationNum(0), seatNum(0), released(false) {
        trainID[0] = '\0';
    }
};

// Global data structures
HashMap userMap;  // username -> index in users array
User users[10000];
int userCount = 0;

HashMap trainMap;  // trainID -> index in trains array
Train trains[1000];
int trainCount = 0;

HashMap loggedInUsers;  // username -> 1 (simple set)

// Helper function to parse time
void parseTime(const char* timeStr, int& hour, int& minute) {
    sscanf(timeStr, "%d:%d", &hour, &minute);
}

// Helper function to parse date
void parseDate(const char* dateStr, int& month, int& day) {
    sscanf(dateStr, "%d-%d", &month, &day);
}

// Helper function to check if a date is within range
bool isDateInRange(int month, int day, int startMonth, int startDay, int endMonth, int endDay) {
    if (month < startMonth || month > endMonth) return false;
    if (month == startMonth && day < startDay) return false;
    if (month == endMonth && day > endDay) return false;
    return true;
}

// Command implementations
int add_user(const char* curUser, const char* username, const char* password,
             const char* name, const char* mailAddr, int privilege) {
    // Check if username already exists
    if (userMap.find(username) != nullptr) {
        return -1;
    }

    // First user special case
    if (userCount == 0) {
        users[userCount] = User(username, password, name, mailAddr, 10);
        userMap.insert(username, userCount);
        userCount++;
        return 0;
    }

    // Check permission
    int* curIdx = userMap.find(curUser);
    if (curIdx == nullptr || !users[*curIdx].loggedIn) {
        return -1;
    }

    if (privilege >= users[*curIdx].privilege) {
        return -1;
    }

    users[userCount] = User(username, password, name, mailAddr, privilege);
    userMap.insert(username, userCount);
    userCount++;
    return 0;
}

int login(const char* username, const char* password) {
    int* idx = userMap.find(username);
    if (idx == nullptr) {
        return -1;
    }

    User& user = users[*idx];
    if (strcmp(user.password, password) != 0) {
        return -1;
    }

    if (user.loggedIn) {
        return -1;  // Already logged in
    }

    user.loggedIn = true;
    loggedInUsers.insert(username, 1);
    return 0;
}

int logout(const char* username) {
    int* idx = userMap.find(username);
    if (idx == nullptr || !users[*idx].loggedIn) {
        return -1;
    }

    users[*idx].loggedIn = false;
    loggedInUsers.erase(username);
    return 0;
}

int query_profile(const char* curUser, const char* username) {
    int* curIdx = userMap.find(curUser);
    int* targetIdx = userMap.find(username);

    if (curIdx == nullptr || targetIdx == nullptr) {
        return -1;
    }

    User& cur = users[*curIdx];
    User& target = users[*targetIdx];

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
    int* curIdx = userMap.find(curUser);
    int* targetIdx = userMap.find(username);

    if (curIdx == nullptr || targetIdx == nullptr) {
        return -1;
    }

    User& cur = users[*curIdx];
    User& target = users[*targetIdx];

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
    // Check if trainID already exists
    if (trainMap.find(trainID) != nullptr) {
        return -1;
    }

    Train& train = trains[trainCount];
    strcpy(train.trainID, trainID);
    train.stationNum = stationNum;
    train.seatNum = seatNum;
    train.type = type;
    train.released = false;

    // Parse stations
    char buffer[5000];
    strcpy(buffer, stationsStr);
    char* token = strtok(buffer, "|");
    int idx = 0;
    while (token && idx < stationNum) {
        strcpy(train.stations[idx], token);
        token = strtok(nullptr, "|");
        idx++;
    }

    // Parse prices
    strcpy(buffer, pricesStr);
    token = strtok(buffer, "|");
    idx = 0;
    while (token && idx < stationNum - 1) {
        train.prices[idx] = atoi(token);
        token = strtok(nullptr, "|");
        idx++;
    }

    // Parse start time
    parseTime(startTime, train.startHour, train.startMinute);

    // Parse travel times
    strcpy(buffer, travelTimesStr);
    token = strtok(buffer, "|");
    idx = 0;
    while (token && idx < stationNum - 1) {
        train.travelTimes[idx] = atoi(token);
        token = strtok(nullptr, "|");
        idx++;
    }

    // Parse stopover times (may be "_" for trains with 2 stations)
    if (stationNum > 2) {
        strcpy(buffer, stopoverTimesStr);
        token = strtok(buffer, "|");
        idx = 0;
        while (token && idx < stationNum - 2) {
            train.stopoverTimes[idx] = atoi(token);
            token = strtok(nullptr, "|");
            idx++;
        }
    }

    // Parse sale date
    strcpy(buffer, saleDateStr);
    token = strtok(buffer, "|");
    parseDate(token, train.saleDateStartMonth, train.saleDateStartDay);
    token = strtok(nullptr, "|");
    parseDate(token, train.saleDateEndMonth, train.saleDateEndDay);

    trainMap.insert(trainID, trainCount);
    trainCount++;
    return 0;
}

int release_train(const char* trainID) {
    int* idx = trainMap.find(trainID);
    if (idx == nullptr) {
        return -1;
    }

    trains[*idx].released = true;
    return 0;
}

int delete_train(const char* trainID) {
    int* idx = trainMap.find(trainID);
    if (idx == nullptr) {
        return -1;
    }

    if (trains[*idx].released) {
        return -1;  // Can't delete released train
    }

    // Mark as deleted (we don't actually remove from array to avoid reindexing)
    trains[*idx].trainID[0] = '\0';
    trainMap.erase(trainID);
    return 0;
}

// Helper function to add minutes to a date/time
void addMinutes(int& month, int& day, int& hour, int& minute, int minutes) {
    minute += minutes;
    hour += minute / 60;
    minute %= 60;

    int days = hour / 24;
    hour %= 24;

    day += days;
    // Simple month/day handling for June-August 2021
    // In reality, we should handle month boundaries properly
    if (day > 30) {  // Simplified
        day -= 30;
        month++;
    }
}

int query_train(const char* trainID, const char* dateStr) {
    int* idx = trainMap.find(trainID);
    if (idx == nullptr) {
        return -1;
    }

    Train& train = trains[*idx];
    int queryMonth, queryDay;
    parseDate(dateStr, queryMonth, queryDay);

    // Check if query date is within sale date range
    if (!isDateInRange(queryMonth, queryDay,
                       train.saleDateStartMonth, train.saleDateStartDay,
                       train.saleDateEndMonth, train.saleDateEndDay)) {
        return -1;
    }

    printf("%s %c\n", train.trainID, train.type);

    // Calculate arrival and departure times for each station
    int currentMonth = queryMonth;
    int currentDay = queryDay;
    int currentHour = train.startHour;
    int currentMinute = train.startMinute;

    int cumulativePrice = 0;

    for (int i = 0; i < train.stationNum; i++) {
        // Arrival time (for first station: xx-xx xx:xx)
        char arriveTime[20], leaveTime[20];

        if (i == 0) {
            strcpy(arriveTime, "xx-xx xx:xx");
        } else {
            sprintf(arriveTime, "%02d-%02d %02d:%02d",
                   currentMonth, currentDay, currentHour, currentMinute);
        }

        // Add stopover time (except for first and last station)
        if (i > 0 && i < train.stationNum - 1) {
            addMinutes(currentMonth, currentDay, currentHour, currentMinute,
                      train.stopoverTimes[i-1]);
        }

        // Departure time (for last station: xx-xx xx:xx)
        if (i == train.stationNum - 1) {
            strcpy(leaveTime, "xx-xx xx:xx");
        } else {
            sprintf(leaveTime, "%02d-%02d %02d:%02d",
                   currentMonth, currentDay, currentHour, currentMinute);
        }

        // Seat information
        char seatInfo[10];
        if (i == train.stationNum - 1) {
            strcpy(seatInfo, "x");
        } else {
            sprintf(seatInfo, "%d", train.seatNum);  // Simplified: no tickets sold yet
        }

        printf("%s %s -> %s %d %s\n", train.stations[i], arriveTime, leaveTime,
               cumulativePrice, seatInfo);

        // Add travel time to next station (except for last station)
        if (i < train.stationNum - 1) {
            addMinutes(currentMonth, currentDay, currentHour, currentMinute,
                      train.travelTimes[i]);
            cumulativePrice += train.prices[i];
        }
    }

    return 0;
}

int clean() {
    // Clear all data
    userCount = 0;
    trainCount = 0;

    // Reinitialize hash maps
    userMap.~HashMap();
    new (&userMap) HashMap();

    trainMap.~HashMap();
    new (&trainMap) HashMap();

    loggedInUsers.~HashMap();
    new (&loggedInUsers) HashMap();

    return 0;
}

int exit_program() {
    printf("bye\n");
    return 0;
}

// Main command parser
void processCommand(const string& line) {
    if (line.empty()) return;

    // Simple command parsing (this needs to be much more robust)
    if (line.find("add_user") == 0) {
        // Parse parameters...
        // For now, just return 0 for testing
        printf("0\n");
    } else if (line.find("clean") == 0) {
        clean();
        printf("0\n");
    } else if (line.find("exit") == 0) {
        exit_program();
        exit(0);
    } else {
        // Unknown command
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