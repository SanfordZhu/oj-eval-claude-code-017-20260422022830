#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace std;

// Very simple implementation to get basic functionality working

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

int clean() {
    userCount = 0;
    trainCount = 0;
    return 0;
}

int exit_program() {
    printf("bye\n");
    return 0;
}

// Simple command parser
void processCommand(const string& line) {
    if (line.empty()) return;

    if (line.find("clean") == 0) {
        printf("%d\n", clean());
    } else if (line.find("exit") == 0) {
        exit_program();
        exit(0);
    } else if (line.find("add_user") == 0) {
        // Parse parameters crudely
        char curUser[25] = "", username[25] = "", password[35] = "", name[16] = "", mailAddr[35] = "";
        int privilege = 0;

        // Very simple parsing - just for testing
        sscanf(line.c_str(), "add_user -c %s -u %s -p %s -n %s -m %s -g %d",
               curUser, username, password, name, mailAddr, &privilege);

        printf("%d\n", add_user(curUser, username, password, name, mailAddr, privilege));
    } else if (line.find("login") == 0) {
        char username[25] = "", password[35] = "";
        sscanf(line.c_str(), "login -u %s -p %s", username, password);
        printf("%d\n", login(username, password));
    } else if (line.find("logout") == 0) {
        char username[25] = "";
        sscanf(line.c_str(), "logout -u %s", username);
        printf("%d\n", logout(username));
    } else if (line.find("query_profile") == 0) {
        char curUser[25] = "", username[25] = "";
        sscanf(line.c_str(), "query_profile -c %s -u %s", curUser, username);
        int result = query_profile(curUser, username);
        if (result == -1) printf("-1\n");
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