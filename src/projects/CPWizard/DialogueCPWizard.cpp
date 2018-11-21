#include "projects/CPWizard/DialogueCPWizard.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>

#include "projects/ContentPatcherProject.hpp"
#include "Program.hpp"

DialogueCPWizard::DialogueCPWizard( ContentPatcherProject& theProj, const std::string& theTarget, xnb::DictionaryData* theDictBase )
:   CPWizard::CPWizard( theProj ),
    target( theTarget ),
    dictBase( * theDictBase )
{
}

void DialogueCPWizard::update()
{
}

void DialogueCPWizard::update( const sf::Event& event )
{
}

void DialogueCPWizard::draw( sf::RenderWindow& window )
{
}
