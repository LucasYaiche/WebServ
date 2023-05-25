#include "../server/Server.hpp"
#include "../parsing/Config.hpp"
#include "../parsing/Location.hpp"
#include "../parsing/ServInfo.hpp"

int main(int argc, char **argv)
{
    if (argc == 2) {
        try {
            // Create a server listening on port 8080
            Config conf(argv[1]);
            std::vector <ServInfo> ports = conf.getConfig();

            Server server(ports);
            server.run();
        }
        catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << "use : ./webserv [path to configuration file]" << std::endl;
        return 1;
    }
    return 0;
}
