//
//DEFINITIONS FOR KUBERNETES OBJECTS AND FUNCTIONS
//
#include <string>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <library.hpp>

using namespace std;

class Node {
    public:
        Node(string name, string status, string role, string age, string version);
        string name;
        string status;
        string role;
        string age;
        string version;
};

Node::Node(string name, string status, string role, string age, string version) {
    this->name = name;
    this->status = status;
    this->role = role;
    this->age = age;
    this->version = version;
}

struct Pod {
    public:
        Pod(string name, int numReady, int totalReady, string status, int numRestarts, string age);
        string name;
        int numReady;
        int totalReady;
        string status;
        int numRestarts;
        string age;
};

Pod::Pod(string name, int numReady, int totalReady, string status, int numRestarts, string age) {
    this->name = name;
    this->numReady = numReady;
    this->totalReady = totalReady;
    this->status = status;
    this->numRestarts = numRestarts;
    this->age = age;
}

class Kube {
    public:
        Kube(string hostname);
        void updateNodes(string hostname);
        void updatePods(string hostname);
        vector<Pod*> podList;
        vector<Node*> nodeList;
};

Kube::Kube(string hostname) {
    updateNodes(hostname);
    updatePods(hostname);
}

void Kube::updateNodes(string hostname) {
    FILE* getNodes = getTerminalOutput(hostname, "kubectl get node");
    char path[PATH_MAX];
	//Discard first row
	fgets(path, PATH_MAX, getNodes);
	while(fgets(path, PATH_MAX, getNodes) != NULL) {
		string stringPath(path);
		stringstream sstreamPath(stringPath);
		string name;
		string status;
		string roles;
		string age;
		string version;

		sstreamPath >> name;
		sstreamPath >> status;
		sstreamPath >> roles;
		sstreamPath >> age;
		sstreamPath >> version;

		//Add new Node to list
		Node* ourNode = new Node(name, status, roles, age, version);
		nodeList.push_back(ourNode); 
	}

}

void Kube::updatePods(string hostname) {
    FILE* getPods = getTerminalOutput(hostname, "kubectl get pods");
    char path[PATH_MAX];

	//Discard first row
	fgets(path, PATH_MAX, getPods);
	while(fgets(path, PATH_MAX, getPods) != NULL) {
		string stringPath(path);
		stringstream sstreamPath(stringPath);
		string name;
		string ready;
		string status;
		int numRestarts;
		int numReady;
		int totalReady;
		string age;

		sstreamPath >> name;
		sstreamPath >> ready;
		sstreamPath >> status;
		sstreamPath >> numRestarts;
		sstreamPath >> age;

		stringstream ssnumReady(ready.substr(0, ready.find("/")));
		ssnumReady >> numReady;

		stringstream sstotalReady(ready.substr(ready.find("/")+1, ready.size()-1));
		sstotalReady >> totalReady;

		Pod* ourPod = new Pod(name, numReady, totalReady, status, numRestarts, age);
		podList.push_back(ourPod);
	}
}

void sendNodesToSlack(string hostname, string channelName) {
	Kube currentKube(hostname);
	vector<Node*> nodeList = currentKube.nodeList;
	slack::post (   
                		"chat.postMessage",
                	{ 
						{"text"      , "*NAME : STATUS : ROLE : AGE : VERSION*" }, 
						{"channel"   , channelName             }
                	}
       				 );

	for (int i = 0; i < nodeList.size(); i++) {
		string currentNodeInfo = nodeList[i]->name.append(" : ").append(nodeList[i]->status).append(" : ").append(nodeList[i]->role).append(" : ").append(nodeList[i]->age).append(" : ").append(nodeList[i]->version);
		slack::post (   
                		"chat.postMessage",
                	{ 
						{"text"      , currentNodeInfo }, 
						{"channel"   , channelName             }
                	}
       				 );
	}
}

void sendPodsToSlack(string hostname, string channelName) {
	Kube currentKube(hostname);
	vector<Pod*> podList = currentKube.podList;
	slack::post (   
                		"chat.postMessage",
                	{ 
						{"text"      , "*NAME : NUMREADY : TOTAL : STATUS : NUMRESTARTS: AGE*" }, 
						{"channel"   , channelName             }
                	}
       				 );

	for (int i = 0; i < podList.size(); i++) {
		string currentPodInfo = podList[i]->name.append(" : ").append(to_string(podList[i]->numReady)).append(" : ").append(to_string(podList[i]->totalReady)).append(" : ").append(podList[i]->status).append(" : ").append(to_string(podList[i]->numRestarts)).append(" : ").append(podList[i]->age);
		slack::post (   
                		"chat.postMessage",
                	{ 
						{"text"      , currentPodInfo }, 
						{"channel"   , channelName             }
                	}
       				 );
	}
}
