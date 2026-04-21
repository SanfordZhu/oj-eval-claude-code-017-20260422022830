#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace std;

// Include command parser
#include "command.h"

// ==================== Constants ====================
const int MAX_USERS = 10000;
const int MAX_TRAINS = 1000;
const int MAX_STATIONS_PER_TRAIN = 100;
const int MAX_ORDERS = 100000;
const int HASH_SIZE = 100007;
const int DAYS_IN_MONTH[13] = {0, 30, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // June-August: 30,31,31

// ==================== Data Structures ====================

// Simple hash table for string keys
template<typename T>
class HashMap {
private:
    struct Node {
        char key[25];
        T value;
        Node* next;
        Node(const char* k, const T& v) : value(v), next(nullptr) {
            strcpy(key, k);
        }
    };

    Node* table[HASH_SIZE];

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
            Node* node = table[i];
            while (node) {
                Node* next = node->next;
                delete node;
                node = next;
            }
        }
    }

    void insert(const char* key, const T& value) {
        unsigned int h = hash(key);
        Node* node = new Node(key, value);
        node->next = table[h];
        table[h] = node;
    }

    T* find(const char* key) {
        unsigned int h = hash(key);
        Node* node = table[h];
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
        Node** pp = &table[h];
        while (*pp) {
            if (strcmp((*pp)->key, key) == 0) {
                Node* to_delete = *pp;
                *pp = to_delete->next;
                delete to_delete;
                return true;
            }
            pp = &(*pp)->next;
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < HASH_SIZE; i++) {
            Node* node = table[i];
            while (node) {
                Node* next = node->next;
                delete node;
                node = next;
            }
            table[i] = nullptr;
        }
    }
};

// ==================== Core Data Types ====================

struct User {
    char username[25];
    char password[35];
    char name[16];
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

struct StationTime {
    int month, day, hour, minute;

    StationTime() : month(0), day(0), hour(0), minute(0) {}
    StationTime(int m, int d, int h, int min) : month(m), day(d), hour(h), minute(min) {}

    bool operator<(const StationTime& other) const {
        if (month != other.month) return month < other.month;
        if (day != other.day) return day < other.day;
        if (hour != other.hour) return hour < other.hour;
        return minute < other.minute;
    }

    void addMinutes(int minutes) {
        minute += minutes;
        hour += minute / 60;
        minute %= 60;
        day += hour / 24;
        hour %= 24;

        // Simple handling for June-August
        while (day > DAYS_IN_MONTH[month]) {
            day -= DAYS_IN_MONTH[month];
            month++;
        }
    }

    string toString(bool showDate = true) const {
        char buf[20];
        if (showDate) {
            sprintf(buf, "%02d-%02d %02d:%02d", month, day, hour, minute);
        } else {
            sprintf(buf, "%02d:%02d", hour, minute);
        }
        return string(buf);
    }
};

struct Train {
    char trainID[25];
    int stationNum;
    char stations[MAX_STATIONS_PER_TRAIN][31];
    int seatNum;
    int prices[MAX_STATIONS_PER_TRAIN - 1];
    int startHour, startMinute;
    int travelTimes[MAX_STATIONS_PER_TRAIN - 1];
    int stopoverTimes[MAX_STATIONS_PER_TRAIN - 2];
    int saleDateStartMonth, saleDateStartDay;
    int saleDateEndMonth, saleDateEndDay;
    char type;
    bool released;

    // Precomputed data for each station
    StationTime arrivalTimes[MAX_STATIONS_PER_TRAIN];
    StationTime departureTimes[MAX_STATIONS_PER_TRAIN];
    int cumulativePrices[MAX_STATIONS_PER_TRAIN];

    Train() : stationNum(0), seatNum(0), released(false) {
        trainID[0] = '\0';
    }

    void precompute() {
        // Compute arrival and departure times for each station
        // Starting from saleDateStart for the first station
        arrivalTimes[0] = StationTime(saleDateStartMonth, saleDateStartDay, startHour, startMinute);
        departureTimes[0] = arrivalTimes[0];
        cumulativePrices[0] = 0;

        for (int i = 1; i < stationNum; i++) {
            // Arrive at station i
            arrivalTimes[i] = departureTimes[i-1];
            arrivalTimes[i].addMinutes(travelTimes[i-1]);

            // Depart from station i (add stopover time except for last station)
            departureTimes[i] = arrivalTimes[i];
            if (i < stationNum - 1) {
                departureTimes[i].addMinutes(stopoverTimes[i-1]);
            }

            // Cumulative price
            cumulativePrices[i] = cumulativePrices[i-1] + prices[i-1];
        }
    }

    // Check if train runs on a specific date (date when leaving from station s)
    bool runsOnDate(int stationIdx, int month, int day) const {
        (void)stationIdx; // Mark as used to avoid warning
        // The train runs daily between sale dates
        // We need to check if there exists a day d such that:
        // departureTimes[stationIdx] on day d equals (month, day, start time)

        // Simplified: check if (month, day) is within sale date range
        // and the train would depart from this station on that day
        if (month < saleDateStartMonth || month > saleDateEndMonth) return false;
        if (month == saleDateStartMonth && day < saleDateStartDay) return false;
        if (month == saleDateEndMonth && day > saleDateEndDay) return false;

        // More precise check would consider travel time from first station
        // For now, return true if within sale date range
        return true;
    }

    // Get departure time from station s on specific date
    StationTime getDepartureTime(int stationIdx, int month, int day) const {
        // Calculate based on first station's departure
        // Days offset = (month, day) - (saleDateStartMonth, saleDateStartDay)
        // Then add travel time to station s

        // Simplified: assume departure time is the same daily pattern
        StationTime result(month, day, startHour, startMinute);
        for (int i = 0; i < stationIdx; i++) {
            result.addMinutes(travelTimes[i]);
            if (i < stationIdx - 1) {
                result.addMinutes(stopoverTimes[i]);
            }
        }
        return result;
    }

    // Get arrival time at station t when departing from station s on date (month, day)
    StationTime getArrivalTime(int fromIdx, int toIdx, int month, int day) const {
        StationTime dep = getDepartureTime(fromIdx, month, day);
        for (int i = fromIdx; i < toIdx; i++) {
            dep.addMinutes(travelTimes[i]);
            if (i < toIdx - 1) {
                dep.addMinutes(stopoverTimes[i]);
            }
        }
        return dep;
    }

    // Get price from station s to t
    int getPrice(int fromIdx, int toIdx) const {
        return cumulativePrices[toIdx] - cumulativePrices[fromIdx];
    }
};

struct Order {
    char username[25];
    char trainID[25];
    int fromIdx, toIdx;
    int numTickets;
    int price;
    StationTime departureTime;
    StationTime arrivalTime;
    int status; // 0: success, 1: pending, 2: refunded
    int timestamp; // For sorting by transaction time

    Order() : fromIdx(0), toIdx(0), numTickets(0), price(0), status(0), timestamp(0) {
        username[0] = '\0';
        trainID[0] = '\0';
    }
};

// ==================== Global State ====================

HashMap<int> userMap;  // username -> index
User users[MAX_USERS];
int userCount = 0;

HashMap<int> trainMap;  // trainID -> index
Train trains[MAX_TRAINS];
int trainCount = 0;

HashMap<bool> loggedInUsers;  // username -> true if logged in

Order orders[MAX_ORDERS];
int orderCount = 0;
int currentTimestamp = 0;

// Simple station index entry
struct StationIndexEntry {
    int trainIdx;
    int stationIdx;
    StationIndexEntry* next;

    StationIndexEntry(int t, int s) : trainIdx(t), stationIdx(s), next(nullptr) {}
};

// Station index: stationName -> linked list of StationIndexEntry
HashMap<StationIndexEntry*> stationIndex;

// ==================== Helper Functions ====================

void parseTime(const char* timeStr, int& hour, int& minute) {
    sscanf(timeStr, "%d:%d", &hour, &minute);
}

void parseDate(const char* dateStr, int& month, int& day) {
    sscanf(dateStr, "%d-%d", &month, &day);
}

int dateToDays(int month, int day) {
    // Convert date to days since June 1
    int days = 0;
    for (int m = 6; m < month; m++) {
        days += DAYS_IN_MONTH[m];
    }
    days += (day - 1);
    return days;
}

bool isDateInRange(int month, int day, int startMonth, int startDay, int endMonth, int endDay) {
    int queryDays = dateToDays(month, day);
    int startDays = dateToDays(startMonth, startDay);
    int endDays = dateToDays(endMonth, endDay);
    return queryDays >= startDays && queryDays <= endDays;
}

// ==================== Command Implementations ====================

int add_user(const char* curUser, const char* username, const char* password,
             const char* name, const char* mailAddr, int privilege) {
    if (userMap.find(username) != nullptr) {
        return -1;
    }

    if (userCount == 0) {
        users[userCount] = User(username, password, name, mailAddr, 10);
        userMap.insert(username, userCount);
        userCount++;
        return 0;
    }

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
        return -1;
    }

    user.loggedIn = true;
    loggedInUsers.insert(username, true);
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

        // Add to station index
        StationIndexEntry** stationList = stationIndex.find(token);
        if (stationList == nullptr) {
            StationIndexEntry* newEntry = new StationIndexEntry(trainCount, idx);
            stationIndex.insert(token, newEntry);
        } else {
            // Add to front of linked list
            StationIndexEntry* newEntry = new StationIndexEntry(trainCount, idx);
            newEntry->next = *stationList;
            *stationList = newEntry;
        }

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

    // Parse stopover times
    if (stationNum > 2) {
        strcpy(buffer, stopoverTimesStr);
        token = strtok(buffer, "|");
        idx = 0;
        while (token && idx < stationNum - 2) {
            if (strcmp(token, "_") != 0) {
                train.stopoverTimes[idx] = atoi(token);
            } else {
                train.stopoverTimes[idx] = 0;
            }
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

    // Precompute arrival/departure times
    train.precompute();

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
        return -1;
    }

    // Remove from station index
    Train& train = trains[*idx];
    for (int i = 0; i < train.stationNum; i++) {
        StationIndexEntry** stationList = stationIndex.find(train.stations[i]);
        if (stationList) {
            // Remove this train from the linked list
            StationIndexEntry** pp = stationList;
            while (*pp) {
                if ((*pp)->trainIdx == *idx) {
                    StationIndexEntry* toDelete = *pp;
                    *pp = toDelete->next;
                    delete toDelete;
                    break;
                }
                pp = &(*pp)->next;
            }
        }
    }

    // Mark as deleted
    train.trainID[0] = '\0';
    trainMap.erase(trainID);
    return 0;
}

int query_train(const char* trainID, const char* dateStr) {
    int* idx = trainMap.find(trainID);
    if (idx == nullptr) {
        return -1;
    }

    Train& train = trains[*idx];
    int queryMonth, queryDay;
    parseDate(dateStr, queryMonth, queryDay);

    if (!isDateInRange(queryMonth, queryDay,
                       train.saleDateStartMonth, train.saleDateStartDay,
                       train.saleDateEndMonth, train.saleDateEndDay)) {
        return -1;
    }

    printf("%s %c\n", train.trainID, train.type);

    // Calculate times for this specific date
    // Days offset from sale start date
    int daysOffset = dateToDays(queryMonth, queryDay) -
                     dateToDays(train.saleDateStartMonth, train.saleDateStartDay);

    for (int i = 0; i < train.stationNum; i++) {
        // Calculate arrival and departure times for this date
        StationTime arrival = train.arrivalTimes[i];
        StationTime departure = train.departureTimes[i];

        // Add days offset
        arrival.day += daysOffset;
        departure.day += daysOffset;

        // Handle month boundaries
        while (arrival.day > DAYS_IN_MONTH[arrival.month]) {
            arrival.day -= DAYS_IN_MONTH[arrival.month];
            arrival.month++;
        }
        while (departure.day > DAYS_IN_MONTH[departure.month]) {
            departure.day -= DAYS_IN_MONTH[departure.month];
            departure.month++;
        }

        char arriveTime[20], leaveTime[20];
        if (i == 0) {
            strcpy(arriveTime, "xx-xx xx:xx");
        } else {
            sprintf(arriveTime, "%02d-%02d %02d:%02d",
                   arrival.month, arrival.day, arrival.hour, arrival.minute);
        }

        if (i == train.stationNum - 1) {
            strcpy(leaveTime, "xx-xx xx:xx");
        } else {
            sprintf(leaveTime, "%02d-%02d %02d:%02d",
                   departure.month, departure.day, departure.hour, departure.minute);
        }

        char seatInfo[10];
        if (i == train.stationNum - 1) {
            strcpy(seatInfo, "x");
        } else {
            sprintf(seatInfo, "%d", train.seatNum); // Simplified: no ticket sales yet
        }

        printf("%s %s -> %s %d %s\n", train.stations[i], arriveTime, leaveTime,
               train.cumulativePrices[i], seatInfo);
    }

    return 0;
}

int query_ticket(const char* fromStation, const char* toStation,
                 const char* dateStr, const char* sortBy) {
    (void)sortBy; // Mark as used to avoid warning
    int queryMonth, queryDay;
    parseDate(dateStr, queryMonth, queryDay);

    // Find all trains that pass through fromStation
    StationIndexEntry** fromList = stationIndex.find(fromStation);
    if (fromList == nullptr) {
        printf("0\n");
        return 0;
    }

    // For each train, check if it also passes through toStation after fromStation
    // Simple array for results
    const int MAX_RESULTS = 1000;
    int resultTrainIdx[MAX_RESULTS];
    int resultFromIdx[MAX_RESULTS];
    int resultToIdx[MAX_RESULTS];
    int resultCount = 0;

    StationIndexEntry* entry = *fromList;
    while (entry && resultCount < MAX_RESULTS) {
        int trainIdx = entry->trainIdx;
        int fromIdx = entry->stationIdx;

        if (trains[trainIdx].trainID[0] == '\0') { // Deleted train
            entry = entry->next;
            continue;
        }
        if (!trains[trainIdx].released) { // Train not released
            entry = entry->next;
            continue;
        }

        // Check if train passes through toStation after fromIdx
        Train& train = trains[trainIdx];
        for (int toIdx = fromIdx + 1; toIdx < train.stationNum; toIdx++) {
            if (strcmp(train.stations[toIdx], toStation) == 0) {
                // Check if train runs on query date
                if (train.runsOnDate(fromIdx, queryMonth, queryDay)) {
                    resultTrainIdx[resultCount] = trainIdx;
                    resultFromIdx[resultCount] = fromIdx;
                    resultToIdx[resultCount] = toIdx;
                    resultCount++;
                }
                break;
            }
        }

        entry = entry->next;
    }

    if (resultCount == 0) {
        printf("0\n");
        return 0;
    }

    // Simple output without sorting for now
    printf("%d\n", resultCount);
    for (int i = 0; i < resultCount; i++) {
        int trainIdx = resultTrainIdx[i];
        int fromIdx = resultFromIdx[i];
        int toIdx = resultToIdx[i];

        Train& train = trains[trainIdx];

        StationTime depTime = train.getDepartureTime(fromIdx, queryMonth, queryDay);
        StationTime arrTime = train.getArrivalTime(fromIdx, toIdx, queryMonth, queryDay);
        int price = train.getPrice(fromIdx, toIdx);

        printf("%s %s %s -> %s %s %d %d\n",
               train.trainID,
               train.stations[fromIdx],
               depTime.toString().c_str(),
               train.stations[toIdx],
               arrTime.toString().c_str(),
               price,
               train.seatNum); // Simplified: full availability
    }

    return 0;
}

int clean() {
    // Clear all data
    userCount = 0;
    trainCount = 0;
    orderCount = 0;
    currentTimestamp = 0;

    // Clear hash maps
    userMap.clear();
    trainMap.clear();
    loggedInUsers.clear();
    stationIndex.clear();

    return 0;
}

int exit_program() {
    printf("bye\n");
    return 0;
}

// ==================== Command Parser ====================

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
        // query_profile prints its own output on success

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
        // modify_profile prints its own output on success

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
        // query_train prints its own output on success

    } else if (parser.command == "query_ticket") {
        const char* fromStation = parser.get("s").c_str();
        const char* toStation = parser.get("t").c_str();
        const char* date = parser.get("d").c_str();
        const char* sortBy = parser.get("p", "time").c_str();

        int result = query_ticket(fromStation, toStation, date, sortBy);
        if (result == -1) {
            printf("-1\n");
        }
        // query_ticket prints its own output

    } else if (parser.command == "clean") {
        int result = clean();
        printf("%d\n", result);

    } else if (parser.command == "exit") {
        exit_program();
        exit(0);

    } else {
        // Unknown command
        printf("-1\n");
    }
}

// ==================== Main Function ====================

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string line;
    while (getline(cin, line)) {
        processCommand(line);
    }

    return 0;
}