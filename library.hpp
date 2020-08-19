#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <slacking.hpp>
#include <vector>
#include <map>
#include <fstream>

using namespace std;

std::string remove_first_last_quote(std::string str) {
    auto s = str;
    if (s.front() == '"' ) {
        s.erase( 0, 1 );
        s.erase(s.size() - 1);
    }
    return s;
}

map<string, vector<string> > generateRuleMap(string ruleFile) {
    map<string, vector<string> > rules;
    
    ifstream ifile(ruleFile);
	
	string myline;
	string command;
	while (getline(ifile, myline)) {
		
		if (myline == "---") {
			getline(ifile, command);
			getline(ifile, myline);
		}

		transform(myline.begin(), myline.end(), myline.begin(), ::tolower); 
		rules[command].push_back(myline);
	}

    return rules;
}

vector<string> readSetupFile(string setupfilename) {
	ifstream ifile(setupfilename);
	if (ifile.fail()) {
		cout << "Couldn't read setup file" << endl;
		exit(1);
	}
	vector<string> setup;
	for (int i = 0; i < 6; i++) {
		string currentline;
		getline(ifile, currentline);
		setup.push_back(currentline.substr(currentline.find("=")+1, currentline.size()-1));
	}
	return setup;
}

void CreateSlackInstance(string botToken) {
    auto& slack = slack::create(botToken);
}

FILE* getTerminalOutput(string hostname, string command) {
    FILE* output;

    //Create string command and convert to cstring
    string ssh = "ssh ";
    command = ssh.append(hostname).append(" '").append(command).append("'");
    char cstrcommand[command.size() + 1];
	strcpy(cstrcommand, command.c_str());
    
    //Run command and generate output
    output = popen(cstrcommand, "r");

    return output;
}

string getLatestMessageText(string CHANNEL_ID) {
	auto getResponse = slack::post(
			"conversations.history",
			{
				{"channel", CHANNEL_ID},
				{"limit", "1"}
			}
		);
	
	if (getResponse["ok"] == "false") {
		return "something went wrong";
	}
	string text = getResponse.at("messages")[0].at("text");
	text = remove_first_last_quote(text);
	return text;
}

double getLatestMessageTimestamp(string CHANNEL_ID) {
	auto getResponse = slack::post(
			"conversations.history",
			{
				{"channel", CHANNEL_ID},
				{"limit", "1"}
			}
		);

	if (getResponse["ok"] == "false") {
		return 0;
	}
	string timestamp = getResponse.at("messages")[0].at("ts");
	timestamp = remove_first_last_quote(timestamp);
	double floatTimestamp = stod(timestamp);
	return floatTimestamp;
}

bool isTimestampLater(double timestamp, double oldtimestamp) {
	if (timestamp > oldtimestamp) {
		return true;
	}
	return false;
}

bool commandExists(map<string, vector<string> > rules, string input) {
    for (map<string, vector<string> >::iterator it = rules.begin(); it != rules.end(); ++it) {
		for (int i = 0; i < it->second.size(); i++) {
			if (it->second[i] == input) {
				return true;
			}
		}
	}
	return false;
}

string generateOutputFromRuleMap(map<string, vector<string> > rules, string input) {
    transform(input.begin(), input.end(), input.begin(), ::tolower); 
	string commandFromRule;
	if (!commandExists(rules, input)) {
		commandFromRule = "UNKNOWN COMMAND";
		return commandFromRule;
	}

	for (map<string, vector<string> >::iterator it = rules.begin(); it != rules.end(); ++it) {
		for (int i = 0; i < it->second.size(); i++) {
			if (it->second[i] == input) {
				commandFromRule = it->first;
			}
		}
	}
	return commandFromRule;
}

void sendMessageToSlack(string CHANNEL_NAME, string text) {
	slack::post (   
                		"chat.postMessage",
                	{ 
						{"text"      , text }, 
						{"channel"   , CHANNEL_NAME             }
                	}
       				 );
}

bool botIsMentioned(string message, string BOT_ID) {
	string firstWord = message.substr(0, message.find(" "));
	if (firstWord == BOT_ID) {
		return true;
	}
	return false;
}

string convertToLower(string str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}



