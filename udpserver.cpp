
// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <unordered_set>
#include <mutex>
#include <thread>
#include "httplib.h"

#define MAXLINE 1024 
//udp port
constexpr int RELAY_PORT = 13370;
//tcp port
constexpr int HTTP_PORT = 13371;

std::string addr_to_string(const in_addr& addr) {
    char client_str[128];
    inet_ntop(AF_INET, &(addr), client_str, INET_ADDRSTRLEN);
    return client_str;

}

std::string sockaddr_to_hostport(const sockaddr_in& addr) {
    return addr_to_string(addr.sin_addr) + ":" + std::to_string(addr.sin_port);
}
//FUCK
std::mutex peers_lock;
std::unordered_set<std::string> peers;
std::string serialize_peers() {
    std::lock_guard lock(peers_lock);
    std::string res = "";
    for(const std::string& p : peers) {
        res += p + "\n";
    }
    res.pop_back();
    return res;
}
void add_peer(const std::string& add) {
    std::lock_guard lock(peers_lock);
    peers.insert(add);
}

bool init_http_server() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(serialize_peers(), "text/plain");
  });
  svr.listen("0.0.0.0", HTTP_PORT);
  return true;
}

int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr, cliaddr; 
       
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
       
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(RELAY_PORT); 
       
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    socklen_t len;
    int n; 

    std::thread t(init_http_server);
    std::cout << "Server starting..." << std::endl;
    while(true) {
        len = sizeof(cliaddr);  //len is value/result 
    
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                    0, ( struct sockaddr *) &cliaddr, 
                    &len); 
        std::string peer = sockaddr_to_hostport(cliaddr);
        std::cout << "client: " << peer << std::endl;
        //add to list of peers
        add_peer(peer);
        sendto(sockfd, buffer, n, 0, (sockaddr*)&cliaddr, sizeof(cliaddr));
    }
    
    return 0; 
}
