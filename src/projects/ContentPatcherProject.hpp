#ifndef CONTENTPATCHERPROJECT_HPP
#define CONTENTPATCHERPROJECT_HPP

#include <SFML/Graphics/Rect.hpp>
#include <string>
#include <vector>
#include <xnb/File.hpp>

#include "projects/ContentPackProject.hpp"
#include "projects/CPWizard/CPWizard.hpp"

class ContentPatcherProject : public ContentPackProject
{
    public:
        struct ContentPatch
        {
            std::string name;
            enum ActionType
            {
                Load = 0,
                EditImage = 1,
                EditData = 2,
            } action;
            std::string target;
            std::string enabled = "true";
            std::vector< std::pair< std::string, std::string > > when;

            // Load, EditImage
            std::string fromFile;

            // EditImage
            sf::IntRect fromArea = sf::IntRect( 0, 0, 0, 0 );
            sf::IntRect toArea = sf::IntRect( 0, 0, 0, 0 );
            enum PatchMode
            {
                Replace = 0,
                Overlay = 1,
            } patchMode = Replace;

            // EditData
            std::vector< std::pair< std::string, std::vector< std::pair< int, std::string > > > > fields;
            std::vector< std::pair< std::string, std::string > > entries;
        };

        struct ConfigEntry
        {
            std::string allowValues;
            bool allowBlank = false;
            bool allowMultiple = false;
            std::string defaults;
        };

        ContentPatcherProject( const std::string& theDir );
        ContentPatcherProject( json theManifest, const std::string& theDir );

        virtual void save() override;

        const std::vector< std::string >& getResourceList() const;
        void addPatch( ContentPatch& patch );
        void closeWizard();

    protected:
        virtual void updateHook() override;
        virtual void eventHook( const sf::Event& event ) override;
        virtual void mainMenuBarHook() override;
        virtual void drawHook( sf::RenderWindow& window ) override;
        virtual void manifestHook() override;

        std::vector< ContentPatch > patches;
        std::vector< ConfigEntry > configs;

        std::vector< std::string > resourceList;
        std::vector< std::string > contentFiles;

        int selPatch = -1;

        enum PatchWizardStage
        {
            NotOpen = 0,
            ChooseFile = 1,
            DoingWizardry,
        } wizardStage = NotOpen;
        std::string wizardFile;
        xnb::File wizardBase;
        std::unique_ptr< CPWizard > activeWizard;

        void refreshResourceList();
        void gatherContentFiles();

        bool doCloseWizard = false;
};

#endif // CONTENTPATCHERPROJECT_HPP
