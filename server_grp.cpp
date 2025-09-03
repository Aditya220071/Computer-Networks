#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <set>
#include <fstream>

using namespace std;
#define PORT 12345

unordered_map<string,string> user_password_map; // for username and password of all clients
int server_fd;

#define BUFFER_SIZE 1024

std::mutex cout_mutex;
std::mutex activeClients_mutex;
std::mutex groups_mutex;

bool starts_with(const string& str, const string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

void handleClient(int user_socket, unordered_map<int,pair<string,string> > &activeClients, unordered_map<string,set<int> > &groups){
    char buffer[BUFFER_SIZE] = {0};

    // sending the message to the client that he has joined the chat
    string message = "Welcome to the chat server!";
    send(user_socket, message.c_str(), message.length(), 0);

    // receiving the message from the client and handling accordingly
    while(true){

        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(user_socket, buffer, BUFFER_SIZE, 0);
        if(valread <= 0){
            break;
        }
        cout_mutex.lock();
        cout<<"Received message: "<<buffer<<" from "<<activeClients[user_socket].first<<endl;
        cout_mutex.unlock();

        string message = buffer;
        if(starts_with(message,"/broadcast")){
            if(message.size() > 11){
                message = message.substr(11);
                // Trim leading spaces
                message.erase(0, message.find_first_not_of(" \t\n\r"));
                
                // Check if the message is empty
                if(message.empty()){
                    message = "Invalid command!"; // Or any other appropriate message
                    send(user_socket, message.c_str(), message.length(), 0);
                }
                else {
                    activeClients_mutex.lock();
                    message = "[" + activeClients[user_socket].first + "]: " + message;
                    for(auto it : activeClients){
                        if(it.first != user_socket){
                            send(it.first, message.c_str(), message.length(), 0);
                        }
                    }
                    activeClients_mutex.unlock();
                }
            }
            else{
                message = "Invalid command!";
                send(user_socket, message.c_str(), message.length(), 0);
            }
        }
        else if(starts_with(message,"/msg")){
            if(message.size() > 5){
                message = message.substr(5);
                string username = message.substr(0,message.find(' '));
                message = message.substr(message.find(' ')+1);
                message.erase(0, message.find_first_not_of(" \t\n\r"));

                // Check if the message is empty
                if(message.empty()){
                    message = "Invalid command!"; // Or any other appropriate message
                    send(user_socket, message.c_str(), message.length(), 0);
                }
                else {
                    activeClients_mutex.lock();
                    message = "[" + activeClients[user_socket].first + "]" + ": " + message;
                    for(auto it : activeClients){
                        if(it.second.first == username){
                            send(it.first, message.c_str(), message.length(), 0);
                            break;
                        }
                    }
                    activeClients_mutex.unlock();
                }
            }
            else{
                message = "Invalid command!";
                send(user_socket, message.c_str(), message.length(), 0);
            }
        }
        else if(starts_with(message,"/create_group")){
            if(message.size() > 14){
                message = message.substr(14);
                message.erase(0, message.find_first_not_of(" \t\n\r"));
                
                // Check if the message is empty
                if(message.empty()){
                    message = "Invalid command!"; // Or any other appropriate message
                    send(user_socket, message.c_str(), message.length(), 0);
                }
                else {
                    groups_mutex.lock();
                    activeClients_mutex.lock();
                    if(groups.find(message) != groups.end()){
                        message = "Group already exists.";
                        send(user_socket, message.c_str(), message.length(), 0);
                    }
                    else if(message.find(' ') != string::npos){
                        message = "Group name should not contain space.";
                        send(user_socket, message.c_str(), message.length(), 0);
                    }
                    else{
                        groups[message].insert(user_socket);
                        cout<<"Group "<<message<<" created"<<endl;
                        message = "Group " + message + " created";
                        send(user_socket, message.c_str(), message.length(), 0);
                    }
                    activeClients_mutex.unlock();
                    groups_mutex.unlock();
                }
            }
            else{
                message = "Invalid command!";
                send(user_socket, message.c_str(), message.length(), 0);
            }
        }
        else if(starts_with(message,"/join_group")){
            if(message.size() > 12){
                message = message.substr(12);
                string group_name = message;
                message.erase(0, message.find_first_not_of(" \t\n\r"));

                // Check if the message is empty
                if(message.empty()){
                    message = "Invalid command!"; // Or any other appropriate message
                    send(user_socket, message.c_str(), message.length(), 0);
                }
                else {
                    groups_mutex.lock();
                    activeClients_mutex.lock();
                    if(groups.find(group_name) == groups.end()){
                        message = "Group does not exist!";
                        send(user_socket, message.c_str(), message.length(), 0);
                    }
                    else if(groups[group_name].find(user_socket) != groups[group_name].end()){
                        message = "You are already inside the group!";
                        send(user_socket, message.c_str(), message.length(), 0);
                    }
                    else{
                        for(auto it : groups[group_name]){
                            message = " has joined the group " + group_name;
                            message = activeClients[user_socket].first + message;
                            if(it != user_socket && activeClients.find(it) != activeClients.end()){
                                send(it, message.c_str(), message.length(), 0);
                            }
                        }
                        groups[group_name].insert(user_socket);
                        message = "You joined the group " + group_name;
                        send(user_socket, message.c_str(), message.length(), 0);
                    }
                    activeClients_mutex.unlock();
                    groups_mutex.unlock();
                }
            }
            else{
                message = "Invalid command!";
                send(user_socket, message.c_str(), message.length(), 0);
            }
        }
        else if(starts_with(message,"/group_msg")){
            if(message.size() > 11){
                message = message.substr(11);

                string group_name = message.substr(0,message.find(' '));
                message = message.substr(message.find(' ')+1);
                message = "[" + group_name + "]" + ": " + message;
                message.erase(0, message.find_first_not_of(" \t\n\r"));

                // Check if the message is empty
                if(message.empty()){
                    message = "Invalid command!"; // Or any other appropriate message
                    send(user_socket, message.c_str(), message.length(), 0);
                }
                else {
                    groups_mutex.lock(); 
                    activeClients_mutex.lock();
                    if(groups.find(group_name) == groups.end()){
                        message = "Group does not exist";
                        send(user_socket, message.c_str(), message.length(), 0);
                    }
                    else if(groups[group_name].find(user_socket) == groups[group_name].end()){
                        message = "You are not part of the group.";
                        send(user_socket, message.c_str(), message.length(), 0);
                    }
                    else {
                        for(auto it : groups[group_name]){
                            if(it != user_socket && activeClients.find(it) != activeClients.end()){
                                send(it, message.c_str(), message.length(), 0);
                            }
                        }
                    }
                    activeClients_mutex.unlock();
                    groups_mutex.unlock();
                }
            }
            else{
                message = "Invalid command!";
                send(user_socket, message.c_str(), message.length(), 0);
            }
        }
        else if(starts_with(message,"/leave_group")){
            if(message.size() > 13){
                message = message.substr(13);
                string group_name = message;
                groups_mutex.lock();
                if(groups.find(group_name) != groups.end() && groups[group_name].find(user_socket) != groups[group_name].end()){
                    message = "You left the group " + group_name;
                    groups[group_name].erase(user_socket);
                    send(user_socket, message.c_str(), message.length(), 0);
                }
                groups_mutex.unlock();
            }
            else{
                message = "Invalid command!";
                send(user_socket, message.c_str(), message.length(), 0);
            }
        }
        else{
            message = "Invalid command";
            send(user_socket, message.c_str(), message.length(), 0);
        }
        
    }

    // deleting the user from active clients
    activeClients_mutex.lock();
    activeClients.erase(user_socket);
    activeClients_mutex.unlock();
}


void start_listening(){
    struct sockaddr_in address;

    // Settign up the address values
    address.sin_family = AF_INET; // for IPv4 addresses
    address.sin_addr.s_addr = INADDR_ANY; // for localhost ***
    address.sin_port = htons(PORT); // for port number

    // Creating the socket for server
    if((server_fd = socket(address.sin_family, SOCK_STREAM, 0)) == 0){
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // checking the ip address by printing boths its binary and human readable form
    cout<<"IP address: "<<inet_ntoa(address.sin_addr)<<endl;
    cout<<"IP address: "<<address.sin_addr.s_addr<<endl;

    // Binding the socket to the address
    // :: is used to specify that we are using the global namespace and differentiate between the bind function and the bind method of the socket class
    if(::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listening for incoming connections
    if(listen(server_fd, 10) > 0){
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout<<"Server is listening on port "<<PORT<<endl;
}

//function to accept incoming request and validate them
void Accept_And_Authenticate_Client(unordered_map<int,pair<string,string> > &activeClients, unordered_map<string,set<int> > &groups){
        int user_socket;
        struct sockaddr_in user_address;
        int addrlen = sizeof(user_address);

        // Accepting the incoming connection
        if((user_socket = accept(server_fd, (struct sockaddr*)&user_address, (socklen_t*)&addrlen)) < 0){
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Authentication
        string username, password;
        string message = "Enter the username: ";
        send(user_socket, message.c_str(), message.length(), 0); // sending the message to request the username

        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        recv(user_socket, buffer, BUFFER_SIZE, 0); // receiving the username
        username = buffer;

        message = "Enter the password: ";
        send(user_socket, message.c_str(), message.length(), 0); // sending the message to request the password

        memset(buffer, 0, BUFFER_SIZE);
        recv(user_socket, buffer, BUFFER_SIZE, 0); // receiving the password
        password = buffer;

        if(user_password_map.find(username) != user_password_map.end() && user_password_map[username] == password){
            activeClients_mutex.lock();
            activeClients[user_socket] = make_pair(username,password);
            // sending message to all client that user has joined
            message = username + " has joined the chat";
            for(auto it : activeClients){
                if(it.first != user_socket){
                    send(it.first, message.c_str(), message.length(),0);
                }
            }
            activeClients_mutex.unlock();
            thread t1([user_socket,&activeClients,&groups](){handleClient(user_socket,activeClients,groups);});
            t1.detach();

        }
        else if(user_password_map.find(username) == user_password_map.end() ){
            message = "No such user exists!";
            send(user_socket, message.c_str(), message.length(), 0);
            close(user_socket);
        }
        else{
            message = "Authentication failed.";
            send(user_socket, message.c_str(), message.length(), 0);
            close(user_socket);
        }
        return;
}

// Function to read username and passwords from users.txt
void Read_UsernameAndPasswords(){
    std::ifstream file("users.txt");  
    if (!file) {
        std::cerr << "Error opening file.\n";
        return;
    }

    std::string line;

    while (std::getline(file, line)) {
        size_t delimiter_pos = line.find(':'); 
        if (delimiter_pos != std::string::npos) {
            std::string username = line.substr(0, delimiter_pos); 
            std::string password = line.substr(delimiter_pos+1); 
            password.erase(password.find_last_not_of(" \t\r\n") + 1);
            user_password_map[username] = password;  
        }
    }

    file.close();
}

int main(){
    Read_UsernameAndPasswords();

    start_listening();
   
    unordered_map<int,pair<string,string> > activeClients; // for client socket and username and password
    unordered_map<string,set<int> > groups; // for group name and set of client sockets


    while(true){
            Accept_And_Authenticate_Client(activeClients,groups);
        }

    return 0;
}