#include "Program.hpp"

#include <boost/filesystem.hpp>
#include <imgui.h>
#include <imgui-sfml.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <SFML/Window/Event.hpp>
#include <tinyfiledialogs.h>
#include <util/Logger.hpp>

#include "projects/ContentPatcherProject.hpp"

using namespace std::placeholders;

sf::RenderWindow Program::window;

void Program::run()
{
    /*
    {
        ProjectType pt;
        pt.displayName = "Json Assets";
        pt.createNewFunc = std::bind( []( const std::string& path ){ return std::unique_ptr< Project >(); }, _1 );
        pt.loadFunc = std::bind( []( const json& manifest, const std::string& path ){ return std::unique_ptr< Project >(); }, _1, _2 );
        projTypes[ "spacechase0.JsonAssets" ] = pt;
    }
    {
        ProjectType pt;
        pt.displayName = "MTN";
        pt.createNewFunc = std::bind( []( const std::string& path ){ return std::unique_ptr< Project >(); }, _1 );
        pt.loadFunc = std::bind( []( const json& manifest, const std::string& path ){ return std::unique_ptr< Project >(); }, _1, _2 );
        projTypes[ "SgtPickles.MTN" ] = pt;
    }
    */
    {
        ProjectType pt;
        pt.displayName = "Content Patcher";
        pt.createNewFunc = std::bind(
        []( const std::string& path )
        {
            return std::unique_ptr< Project >( new ContentPatcherProject( path ) );
        }, _1 );
        pt.loadFunc = std::bind(
        []( const json& manifest, const std::string& path )
        {
            return std::unique_ptr< Project >( new ContentPatcherProject( manifest, path ) );
        }, _1, _2 );
        projTypes[ "Pathoschild.ContentPatcher" ] = pt;
    }

    window.create( sf::VideoMode( 800, 600 ), "Stardew Editor 2" );
    window.setVerticalSyncEnabled( true );
    ImGui::SFML::Init( window );

    sf::Clock clock;
    while ( isRunning )
    {
        sf::Event event;
        while ( window.pollEvent( event ) )
        {
            ImGui::SFML::ProcessEvent( event );
            if ( currProj )
                currProj->eventHook( event );
            if ( event.type == sf::Event::Closed )
                isRunning = false;
        }
        ImGui::SFML::Update( window, clock.restart() );

        //ImGui::ShowDemoWindow(nullptr);

        if ( ImGui::BeginMainMenuBar() )
        {
            if ( ImGui::BeginMenu( "File" ) )
            {
                if ( ImGui::BeginMenu( "New" ) )
                {
                    for ( const auto& entry : projTypes )
                    {
                        if ( ImGui::MenuItem( ( entry.second.displayName + " content pack" ).c_str() ) )
                        {
                            const char* path_ = tinyfd_selectFolderDialog( "Select empty folder for new project", fs::current_path().string().c_str() );
                            std::string path = path_ == nullptr ? std::string() : std::string( path_ );

                            bool foundFiles = false;
                            for ( fs::directory_iterator it( path ); it != fs::directory_iterator(); ++it )
                            {
                                foundFiles = true;
                                break;
                            }

                            if ( foundFiles )
                            {
                                tinyfd_messageBox( "Directory must be empty", "The directory must be empty!", "ok", "error", 1 );
                            }
                            else
                            {
                                currProj = entry.second.createNewFunc( path );
                            }
                        }
                    }
                    ImGui::EndMenu();
                }
                if ( ImGui::MenuItem( "Open" ) )
                {
                    const char* filters[] = { "manifest.json" };
                    const char* path_ = tinyfd_openFileDialog( "Open", fs::current_path().string().c_str(), 1, filters, "Manifest for content pack", false );
                    std::string path = path_ == nullptr ? std::string() : std::string( path_ );
                    if ( path != "" )
                    {
                        json manifest;
                        try
                        {
                            std::ifstream file( path );
                            file >> manifest;
                            file.close();
                        }
                        catch ( std::exception& e )
                        {
                            util::log( "Exception parsing json: $\n", e.what() );
                        }

                        if ( manifest.has_key( "EntryDll" ) )
                        {
                            tinyfd_messageBox( "Error", "You don't really expect me to handle SMAPI mods, do you?", "ok", "error", 1 );
                        }
                        else
                        {
                            std::string mod = manifest[ "ContentPackFor" ][ "UniqueID" ].as< std::string >();
                            std::string dir = fs::path( path ).remove_filename().string();

                            currProj.reset();
                            for ( const auto& entry : projTypes )
                            {
                                if ( mod == entry.first )
                                {
                                    currProj = entry.second.loadFunc( manifest, dir );
                                }
                            }

                            if ( !currProj )
                            {
                                tinyfd_messageBox( "Unsupported content pack type",
                                                   util::format( "We don't handle content packs for the mod '$'.", mod ).c_str(),
                                                   "ok", "info", 1 );
                            }
                        }
                    }
                }
                if ( ImGui::MenuItem( "Save", nullptr, false, currProj != nullptr ) )
                {
                    currProj->save();
                }
                if ( currProj )
                    currProj->fileMenuHook();
                ImGui::EndMenu();
            }
            if ( currProj )
                currProj->mainMenuBarHook();
            ImGui::EndMainMenuBar();
        }

        if ( currProj )
            currProj->updateHook();
        ImGui::EndFrame();

        window.clear( sf::Color::White );
        if ( currProj )
            currProj->drawHook( window );
        ImGui::SFML::Render( window );
        window.display();
    }
}
