#include "projects/ContentPackProject.hpp"

#include <boost/filesystem.hpp>
#include <iomanip>

#include "imgui.h"

ContentPackProject::ContentPackProject( const std::string cpFor, const std::string& theDir )
:   dir( theDir )
{
    manifest[ "Name" ] = "YourProjectName";
    manifest[ "Author" ] = "your name";
    manifest[ "Version" ] = "1.0.0";
    manifest[ "Description" ] = "One or two sentences about the mod.";
    manifest[ "UniqueID" ] = "YourName.YourProjectName";
    manifest[ "MinimumApiVersion" ] = "2.0",
    manifest[ "UpdateKeys" ] = json::array( { "Nexus:" } );
    manifest[ "ContentPackFor" ][ "UniqueID" ] = "Pathoschild.ContentPatcher";
}

ContentPackProject::ContentPackProject( json theManifest, const std::string& theDir )
:   manifest( theManifest ),
    dir( theDir )
{
}

void ContentPackProject::save()
{
    util::log( "Saving content pack manifest..." );
    std::ofstream file( ( fs::path( dir ) / "manifest.json" ).string() );
    file << pretty_print( manifest );
    file.close();
}

void ContentPackProject::updateHook()
{
    if ( ImGui::Begin( "Manifest", nullptr ) )
    {
        std::string str = manifest[ "Name" ].as< std::string >();
        str.resize( 256, '\0' );
        ImGui::InputText( "Name", &str[ 0 ], 256 );
        manifest[ "Name" ] = str.c_str();

        str = manifest[ "Author" ].as< std::string >();
        str.resize( 256, '\0' );
        ImGui::InputText( "Author", &str[ 0 ], 256 );
        manifest[ "Author" ] = str.c_str();

        str = manifest[ "Version" ].as< std::string >();
        str.resize( 256, '\0' );
        ImGui::InputText( "Version", &str[ 0 ], 256 );
        manifest[ "Version" ] = str.c_str();

        str = manifest[ "Description" ].as< std::string >();
        str.resize( 256, '\0' );
        ImGui::InputText( "Description", &str[ 0 ], 256 );
        manifest[ "Description" ] = str.c_str();

        str = manifest[ "UniqueID" ].as< std::string >();
        str.resize( 256, '\0' );
        ImGui::InputText( "Unique ID", &str[ 0 ], 256 );
        manifest[ "UniqueID" ] = str.c_str();

        str = manifest[ "UpdateKeys" ][ 0 ].as< std::string >();
        str.resize( 256, '\0' );
        ImGui::InputText( "Update Key", &str[ 0 ], 256 );
        manifest[ "UpdateKeys" ][ 0 ] = str.c_str();

        /*
        doJsonImguiTextField( manifest[ "Author" ], "Author" );
        doJsonImguiTextField( manifest[ "Version" ], "Version" );
        doJsonImguiTextField( manifest[ "Description" ], "Description" );
        doJsonImguiTextField( manifest[ "UniqueID" ], "Unique ID" );
        doJsonImguiTextField( manifest[ "UpdateKeys" ][ 0 ], "Update Key" );
        */

        manifestHook();
    }
    ImGui::End();
}

void ContentPackProject::manifestHook()
{
}
