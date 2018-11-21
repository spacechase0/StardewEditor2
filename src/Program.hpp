#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <functional>
#include <map>
#include <memory>
#include <SFML/Graphics/RenderWindow.hpp>
#include <string>

#include "Project.hpp"

class Program
{
    public:
        void run();

        static sf::RenderWindow window;

    private:

        bool isRunning = true;

        struct ProjectType
        {
            std::string displayName;
            std::function< std::unique_ptr<Project> ( const std::string& ) > createNewFunc;
            std::function< std::unique_ptr<Project> ( const json&, const std::string& ) > loadFunc;
        };

        std::unique_ptr<Project> currProj;
        std::map<std::string, ProjectType > projTypes;
};

#endif // PROGRAM_HPP
