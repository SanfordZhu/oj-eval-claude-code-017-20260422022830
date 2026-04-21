#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <cstring>
#include <map>

using namespace std;

class CommandParser {
private:
    map<string, string> params;

    void parseLine(const string& line) {
        params.clear();

        size_t pos = line.find(' ');
        if (pos == string::npos) return;

        string cmd = line.substr(0, pos);
        string args = line.substr(pos + 1);

        size_t start = 0;
        while (start < args.length()) {
            // Find next parameter
            if (args[start] == '-') {
                size_t key_end = args.find(' ', start + 1);
                if (key_end == string::npos) key_end = args.length();

                string key = args.substr(start + 1, key_end - start - 1);

                // Find value
                start = key_end + 1;
                if (start >= args.length()) {
                    params[key] = "";
                    break;
                }

                size_t value_end = args.find(' ', start);
                if (value_end == string::npos) value_end = args.length();

                string value = args.substr(start, value_end - start);
                params[key] = value;

                start = value_end + 1;
            } else {
                start++;
            }
        }
    }

public:
    string command;

    CommandParser() {}

    void parse(const string& line) {
        size_t pos = line.find(' ');
        if (pos == string::npos) {
            command = line;
            params.clear();
        } else {
            command = line.substr(0, pos);
            parseLine(line);
        }
    }

    string get(const string& key, const string& defaultValue = "") {
        auto it = params.find(key);
        if (it != params.end()) {
            return it->second;
        }
        return defaultValue;
    }

    int getInt(const string& key, int defaultValue = 0) {
        string val = get(key);
        if (val.empty()) return defaultValue;
        return atoi(val.c_str());
    }

    bool has(const string& key) {
        return params.find(key) != params.end();
    }

    void clear() {
        command.clear();
        params.clear();
    }
};

#endif // COMMAND_H