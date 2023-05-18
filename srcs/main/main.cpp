#include "../server/Server.hpp"

int main() {
    // Create a server listening on port 8080
    Server server("127.0.0.1",8080);

    // Set the document root directory
    

    // Start the server
    server.run();

    return 0;
}
