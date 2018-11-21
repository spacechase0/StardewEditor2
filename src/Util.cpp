#include "Util.hpp"

#include <imgui.h>
#include <util/File.hpp>

#ifdef SFML_SYSTEM_WINDOWS
#include <windows.h>

std::string getRegistryValue( const std::string& key, const std::string& value )
{
    HKEY hkey;
    auto err = RegOpenKeyEx( HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hkey );
    if ( err != ERROR_SUCCESS )
        return "";

    unsigned long size = 512;
    unsigned char buffer[ size ];
    err = RegQueryValueEx( hkey, value.c_str(), 0, nullptr, &buffer[ 0 ], &size );
    RegCloseKey( hkey );

    std::string ret = reinterpret_cast< char* >( buffer );
    return ret;
}
#endif

std::string getStardewContentsDirectory()
{
    #if defined( SFML_SYSTEM_WINDOWS )
    if ( fs::exists( "C:\\Program Files (x86)\\GalaxyClient\\Games\\Stardew Valley\\Content" ) )
        return "C:\\Program Files (x86)\\GalaxyClient\\Games\\Stardew Valley\\Content";
    else if ( fs::exists( "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Stardew Valley\\Content" ) )
        return "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Stardew Valley\\Content";
    else if ( fs::exists( "C:\\Program Files\\GalaxyClient\\Games\\Stardew Valley\\Content" ) )
        return "C:\\Program Files\\GalaxyClient\\Games\\Stardew Valley\\Content";
    else if ( fs::exists( "C:\\Program Files\\GOG Galaxy\\Games\\Stardew Valley\\Content" ) )
        return "C:\\Program Files\\GOG Galaxy\\Games\\Stardew Valley\\Content";
    else if ( fs::exists( "C:\\Program Files\\Steam\\steamapps\\common\\Stardew Valley\\Content" ) )
        return "C:\\Program Files\\Steam\\steamapps\\common\\Stardew Valley\\\Content";
    else
    {
        std::string steam = getRegistryValue( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App 413150", "InstallLocation" );
        std::string gog = getRegistryValue( "SOFTWARE\\WOW6432Node\\GOG.com\\Games\\1453375253", "PATH" );
        if ( steam != "" && fs::exists( steam ) )
            return steam + "\\Content";
        else if ( gog != "" && fs::exists( gog ) )
            return gog + "\\Content";
    }
    #elif defined( SFML_SYSTEM_MACOS )
        std::string home = std::getenv( "HOME" );
        if ( fs::exists( "/Applications/Stardew Valley.app/Contents/MacOS/Content" ) )
            return "/Applications/Stardew Valley.app/Contents/MacOS/Content";
        else if ( fs::exists( home + "/Library/Application Support/Steam/steamapps/common/Stardew Valley/Contents/MacOS/Content" ) )
            return home + "/Library/Application Support/Steam/steamapps/common/Stardew Valley/Contents/MacOS/Content";
    #else
        if ( fs::exists( home + "/GOG Games/Stardew Valley/game/Content" ) )
            return home + "/GOG Games/Stardew Valley/game/Content";
        else if ( fs::exists( home + "/.steam/steam/steamapps/common/Stardew Valley/Content" ) )
            return home + "/.steam/steam/steamapps/common/Stardew Valley/Content";
        else if ( fs::exists( home + "/.local/share/Steam/steamapps/common/Stardew Valley/Content" ) )
            return home + "/.local/share/Steam/steamapps/common/Stardew Valley/Content";
    #endif
    return "./Content";
}

void doStringImguiTextField(std::string& str, const std::string& displayName)
{
    std::string data = str;
    data.resize( 256 );
    ImGui::InputText( displayName.c_str(), &data[ 0 ], 256 );
    str = data.c_str();
}

// Stolen from the demo file
void showImguiHelp( const std::string& str )
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(str.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// https://stackoverflow.com/a/10167550/1687492
template < >
fs::path& fs::path::append< typename fs::path::iterator >( typename fs::path::iterator begin, typename fs::path::iterator end )
{
    for( ; begin != end ; ++begin )
        *this /= *begin;
    return *this;
}
// Return path when appended to a_From will resolve to same as a_To
boost::filesystem::path make_relative( boost::filesystem::path a_From, boost::filesystem::path a_To )
{
    a_From = boost::filesystem::absolute( a_From ); a_To = boost::filesystem::absolute( a_To );
    boost::filesystem::path ret;
    boost::filesystem::path::const_iterator itrFrom( a_From.begin() ), itrTo( a_To.begin() );
    // Find common base
    for( boost::filesystem::path::const_iterator toEnd( a_To.end() ), fromEnd( a_From.end() ) ; itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo; ++itrFrom, ++itrTo );
    // Navigate backwards in directory to reach previously found base
    for( boost::filesystem::path::const_iterator fromEnd( a_From.end() ); itrFrom != fromEnd; ++itrFrom )
    {
        if( (*itrFrom) != "." )
            ret /= "..";
    }
    // Now navigate down the directory branch
    ret.append( itrTo, a_To.end() );
    return ret;
}
