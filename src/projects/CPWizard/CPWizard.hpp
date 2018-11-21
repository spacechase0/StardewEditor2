#ifndef CPWIZARD_HPP
#define CPWIZARD_HPP

namespace sf
{
    class Event;
    class RenderWindow;
}

class ContentPatcherProject;

class CPWizard
{
    public:
        CPWizard( ContentPatcherProject& theProj );
        virtual ~CPWizard();

        virtual void update();
        virtual void update(const sf::Event& event);
        virtual void draw( sf::RenderWindow& window );

    protected:
        ContentPatcherProject& proj;
};

#endif // CPWIZARD_HPP
