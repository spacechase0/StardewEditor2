#ifndef DIALOGUECPWIZARD_HPP
#define DIALOGUECPWIZARD_HPP

#include <xnb/DictionaryType.hpp>

#include "projects/CPWizard/CPWizard.hpp"

class DialogueCPWizard : public CPWizard
{
    public:
        DialogueCPWizard( ContentPatcherProject& theProj, const std::string& theTarget, xnb::DictionaryData* theDictBase );

        virtual void update() override;
        virtual void update( const sf::Event& event ) override;
        virtual void draw( sf::RenderWindow& window ) override;

        struct Dialogue
        {
            enum Type
            {
                Normal,
                GenderSpecific,
                Plain,

                Pause,
            };

            int portaitId = 0;

            std::string message;
        };

    private:
        const std::string target;
        xnb::DictionaryData& dictBase;

        std::vector< Dialogue > dialogueStack;
};

#endif // DIALOGUECPWIZARD_HPP

