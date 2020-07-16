#include <iostream>
#include <slacking.hpp>
#include <kubernetes.hpp>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>



void sendHelpToSlack(string CHANNEL_NAME) {
    sendMessageToSlack(CHANNEL_NAME, "Here are a list of valid commands");
    sendMessageToSlack(CHANNEL_NAME, "`@kubernetesbot get nodes` show's information about nodes");
    sendMessageToSlack(CHANNEL_NAME, "`@kubernetesbot get pods`   shows information about pods");
    sendMessageToSlack(CHANNEL_NAME, "`@kubernetesbot help`   displays valid commands");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
		printf("Usage: ./kubernetesbot \"setup.txt\"");
		cout << endl;
		exit(1);
	}
    vector<string> setup = readSetupFile(argv[1]);
    string HOSTNAME = setup[0]; string CHANNEL_ID = setup[1]; string CHANNEL_NAME = setup[2]; 
    string BOT_TOKEN = setup[3]; string BOT_ID = setup[4]; string RULES = setup[5];

    cout << HOSTNAME << endl << CHANNEL_ID << endl << CHANNEL_NAME << endl << BOT_TOKEN << endl << BOT_ID << endl << RULES << endl;

    map<string, vector <string> > rulesMap = generateRuleMap(RULES);
    CreateSlackInstance(BOT_TOKEN);

    double previousTimeStamp = 0;
    while (true) {
        double newTimeStamp = getLatestMessageTimestamp(CHANNEL_ID);
        string message = getLatestMessageText(CHANNEL_ID);
        if (botIsMentioned(message, BOT_ID) && isTimestampLater(newTimeStamp, previousTimeStamp)) {
            message = message.substr(message.find(" ") + 1, message.size()-1);
            message = convertToLower(message);
            if (message == "get nodes") {
                sendNodesToSlack(HOSTNAME, CHANNEL_NAME);
            }
            else if (message == "get pods") {
                sendPodsToSlack(HOSTNAME, CHANNEL_NAME);
            }

            else if (message == "help") {
                sendHelpToSlack(CHANNEL_NAME);
            }

            else {
                string output = generateOutputFromRuleMap(rulesMap, message);
                if (output == "UNKNOWN COMMAND") {
                    sendHelpToSlack(CHANNEL_NAME);
                }
                else {
                    sendMessageToSlack(CHANNEL_NAME, output);
                }
            }
        }
        previousTimeStamp = newTimeStamp;
        std::this_thread::sleep_for(chrono::seconds(1));

    }

}