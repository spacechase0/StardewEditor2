#ifndef PROJECT_HPP
#define PROJECT_HPP

namespace sf
{
    class Event;
    class RenderWindow;
}
class Program;

class Project
{
    public:
        virtual ~Project();

        virtual void save() = 0;

    protected:
        friend class Program;

        virtual void fileMenuHook();
        virtual void mainMenuBarHook();
        virtual void updateHook();
        virtual void eventHook( const sf::Event& event );
        virtual void drawHook( sf::RenderWindow& window );

};

#endif // PROJECT_HPP
