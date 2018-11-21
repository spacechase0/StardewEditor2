#include "projects/ContentPatcherProject.hpp"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <imgui.h>
#include <tinyfiledialogs.h>
#include <xnb/DictionaryType.hpp>
#include <xnb/TextureType.hpp>

#include "projects/CPWizard/DialogueCPWizard.hpp"
#include "projects/CPWizard/ImageCPWizard.hpp"

ContentPatcherProject::ContentPatcherProject( const std::string& theDir )
:   ContentPackProject::ContentPackProject( std::string( "Pathoschild.ContentPatcher" ), theDir )
{
    gatherContentFiles();
}

ContentPatcherProject::ContentPatcherProject( json theManifest, const std::string& theDir )
:   ContentPackProject::ContentPackProject( theManifest, theDir )
{
    gatherContentFiles();
    if ( manifest[ "ContentPackFor" ][ "UniqueID" ] != "Pathoschild.ContentPatcher" )
        throw std::invalid_argument( "Must be a content patcher pack!" );
    refreshResourceList();

    try
    {
        util::log( "Parsing content.json...\n" );
        json content;
        std::ifstream file( ( fs::path( dir ) / "content.json" ).string() );
        file >> content;
        file.close();

        util::log( "Patch count: $\n", content[ "Changes" ].size() );
        int i = 1;
        for ( const json& jpatch : content[ "Changes" ].array_range() )
        {
            util::log( "\tDoing patch $...\n", i++ );
            ContentPatch patch;
            patch.name    = jpatch.has_key( "LogName" ) ? jpatch[ "LogName" ].as< std::string >() : "";
            patch.target  = jpatch[ "Target" ].as< std::string >();
            patch.enabled = jpatch.has_key( "Enabled" ) ? jpatch[ "Enabled" ].as< std::string >() : "true";
            // TODO: When

            std::string action = jpatch[ "Action" ].as< std::string >();
                 if ( action == "Load"      ) patch.action = ContentPatch::ActionType::Load;
            else if ( action == "EditImage" ) patch.action = ContentPatch::ActionType::EditImage;
            else if ( action == "EditData"  ) patch.action = ContentPatch::ActionType::EditData;

            if ( patch.action == ContentPatch::ActionType::Load || patch.action == ContentPatch::ActionType::EditImage )
            {
                patch.fromFile = jpatch[ "FromFile" ].as< std::string >();
            }
            if ( patch.action == ContentPatch::ActionType::EditImage )
            {
                if ( jpatch.has_key( "FromArea" ) )
                {
                    patch.fromArea = sf::IntRect( jpatch[ "FromArea" ][ "X"     ].as< int >(), jpatch[ "FromArea" ][ "Y"      ].as< int >(),
                                                  jpatch[ "FromArea" ][ "Width" ].as< int >(), jpatch[ "FromArea" ][ "Height" ].as< int >() );
                    patch.toArea   = sf::IntRect( jpatch[ "ToArea" ][ "X"     ].as< int >(), jpatch[ "ToArea" ][ "Y"      ].as< int >(),
                                                  jpatch[ "ToArea" ][ "Width" ].as< int >(), jpatch[ "ToArea" ][ "Height" ].as< int >() );
                }
                if ( jpatch.has_key( "PatchMode" ) )
                {
                    if ( jpatch[ "PatchMode" ] == "Overlay" )
                        patch.patchMode = ContentPatch::PatchMode::Overlay;
                }
            }
            if ( patch.action == ContentPatch::ActionType::EditData )
            {
                if ( jpatch.has_key( "Fields" ) )
                {
                    for ( const auto& entry : jpatch[ "Fields" ].object_range() )
                    {
                        std::pair< std::string, std::vector< std::pair< int, std::string > > > out;
                        out.first = static_cast< std::string >( entry.key() );
                        for ( const auto& field : entry.value().object_range() )
                        {
                            out.second.push_back( std::make_pair( util::fromString< int >( static_cast< std::string >( field.key() ) ), field.value().as< std::string >() ) );
                        }
                        patch.fields.push_back( out );
                    }
                }
                if ( jpatch.has_key( "Entries" ) )
                {
                    for ( const auto& entry : jpatch[ "Entries" ].object_range() )
                    {
                        patch.entries.push_back( std::make_pair( static_cast< std::string >( entry.key() ), entry.value().as< std::string >() ) );
                    }
                }
            }

            patches.push_back( patch );
        }
    }
    catch ( std::exception& e )
    {
        util::log( "Exception while parsing content.json: $\n", e.what() );
    }
}

void ContentPatcherProject::save()
{
    ContentPackProject::save();

    util::log( "Saving Content Patcher content.json..." );

    json j;
    j[ "Format" ] = "1.4";

    json::array changes;
    for ( const ContentPatch& patch : patches )
    {
        json jpatch;

        std::string action = "";
        switch ( patch.action )
        {
            case ContentPatch::ActionType::Load:
                {
                    action = "Load";
                    jpatch[ "FromFile" ] = patch.fromFile;
                }
                break;
            case ContentPatch::ActionType::EditImage:
                {
                    action = "EditImage";
                    jpatch[ "FromFile" ] = patch.fromFile;
                    json fromArea;
                    fromArea[ "X" ] = patch.fromArea.left;
                    fromArea[ "Y" ] = patch.fromArea.top;
                    fromArea[ "Width" ] = patch.fromArea.width;
                    fromArea[ "Height" ] = patch.fromArea.height;
                    jpatch[ "FromArea" ] = fromArea;
                    json toArea;
                    toArea[ "X" ] = patch.toArea.left;
                    toArea[ "Y" ] = patch.toArea.top;
                    toArea[ "Width" ] = patch.toArea.width;
                    toArea[ "Height" ] = patch.toArea.height;
                    jpatch[ "ToArea" ] = toArea;
                    if ( patch.patchMode == ContentPatch::PatchMode::Overlay )
                        jpatch[ "PatchMode" ] = "Overlay";
                }
                break;
            case ContentPatch::ActionType::EditData:
                {
                    action = "EditData";

                    if ( patch.fields.size() > 0 )
                    {
                        json fields;
                        for ( const auto& row : patch.fields )
                        {
                            json jrow;
                            for ( const auto& field : row.second )
                                jrow[ util::toString( field.first ) ] = field.second;
                            fields[ row.first ] = jrow;
                        }
                        jpatch[ "Fields" ] = fields;
                    }

                    if ( patch.entries.size() > 0 )
                    {
                        json entries;
                        for ( const auto& entry : patch.entries )
                        {
                            entries[ entry.first ] = entry.second;
                        }
                        jpatch[ "Entries" ] = entries;
                    }
                }
                break;
        }

        jpatch[ "Action" ] = action;
        jpatch[ "Target" ] = patch.target;
        if ( patch.name != "" )
            jpatch[ "LogName" ] = patch.name;
        if ( patch.enabled != "true" )
            jpatch[ "Enabled" ] = patch.enabled;

        if ( patch.when.size() > 0 )
        {
        json conds;
        for ( const auto& cond : patch.when )
            conds[ cond.first ] = cond.second;
        jpatch[ "When" ] = conds;
        }

        changes.push_back( jpatch );
    }
    j[ "Changes" ] = changes;

    if ( configs.size() > 0 )
    {
        json::array schema;
        for ( const ConfigEntry& config : configs )
        {
            json cs;

            cs[ "AllowValues" ] = config.allowValues;
            if ( config.allowBlank != false )
                cs[ "AllowBlank" ] = "true";
            if ( config.allowMultiple != false )
                cs[ "AllowMultiple" ] = "true";
            if ( config.defaults != "" )
                cs[ "Default" ] = config.defaults;

            schema.push_back( cs );
        }
        j[ "ConfigSchema" ] = schema;
    }

    std::ofstream file( ( fs::path( dir ) / "content.json" ).string() );
    file << pretty_print( j );
    file.close();
}

const std::vector< std::string >& ContentPatcherProject::getResourceList() const
{
    return resourceList;
}

void ContentPatcherProject::addPatch( ContentPatch& patch )
{
    selPatch = patches.size();
    patches.push_back( patch );
}

void ContentPatcherProject::closeWizard()
{
    doCloseWizard = true;
}

void ContentPatcherProject::updateHook()
{
    ContentPackProject::updateHook();

    if ( selPatch != -1 )
    {
        ContentPatch& patch = patches[ selPatch ];
        bool open = true;
        if ( ImGui::Begin( "Content Patch", &open ) )
        {
            if ( !open )
                selPatch = -1;

            if ( ImGui::Button( "Delete this patch" ) )
            {
                patches.erase( patches.begin() + selPatch );
                selPatch = -1;
                ImGui::End();
                return;
            }

            doStringImguiTextField( patch.name, "Log Name" );
            showImguiHelp( "A name for this patch shown in log messages." );

            const char* items[] = { "Load", "Edit Image", "Edit Data" };
            ImGui::Combo( "Action", reinterpret_cast< int* >( &patch.action ), items, 3 );
            showImguiHelp("The kind of change to make." );

            doStringImguiTextField( patch.target, "Target" );
            showImguiHelp( "The game asset you want to patch. This is the file path inside your game's `Content` folder, without the file extension or language." );
            doStringImguiTextField( patch.enabled, "Enabled" );
            showImguiHelp( "Whether to apply this patch." );

            ImGui::Text( "When" );
            showImguiHelp( "Only apply the patch if the given conditions match." );
            for ( std::size_t ic = 0; ic < patch.when.size(); ++ic )
            {
                doStringImguiTextField( patch.when[ ic ].first, "Condition" );
                doStringImguiTextField( patch.when[ ic ].second, "Value" );

                if ( ImGui::Button( util::format( "Remove Condition##when$", ic ).c_str() ) )
                {
                    patch.when.erase( patch.when.begin() + ic );
                    break;
                }
            }
            if ( ImGui::Button( "Add Condition" ) )
            {
                patch.when.push_back( std::pair< std::string, std::string >() );
            }

            ImGui::Separator();

            if ( patch.action == ContentPatch::ActionType::Load )
            {
                doStringImguiTextField( patch.fromFile, "From File" );
                showImguiHelp( "The relative file path in your content pack folder to load instead." );
            }
            if ( patch.action == ContentPatch::ActionType::EditImage )
            {
                doStringImguiTextField( patch.fromFile, "From File" );
                showImguiHelp( "The relative path to the image in your content pack folder to patch into the target." );

                ImGui::InputInt2( "Patch Size", &patch.fromArea.width );
                showImguiHelp( "The size of the patch." );
                patch.toArea.width = patch.fromArea.width;
                patch.toArea.height = patch.fromArea.height;
                ImGui::InputInt2( "Patch From", &patch.fromArea.left );
                showImguiHelp( "The part of the source image to copy." );
                ImGui::InputInt2( "Patch To", &patch.toArea.left );
                showImguiHelp( "The part of the target image to patch over." );

                const char* items2[] = { "Replace", "Overlay" };
                ImGui::Combo( "Patch Mode", reinterpret_cast< int* >( &patch.patchMode ), items2, 2 );
                showImguiHelp( "How to apply the patch." );
            }
            if ( patch.action == ContentPatch::ActionType::EditData )
            {
                ImGui::Text( "Fields" );
                showImguiHelp( "The individual fields you want to change for existing entries." );
                //ImGui::BeginChild( "fields" );
                {
                    for ( std::size_t ik = 0; ik < patch.fields.size(); ++ik )
                    {
                        doStringImguiTextField( patch.fields[ ik ].first, util::format( "Key##fieldkey$", ik ) );
                        if ( ImGui::Button( util::format( "Remove key##$", ik ).c_str() ) )
                        {
                            patch.fields.erase( patch.fields.begin() + ik );
                            break;
                        }
                        for ( std::size_t iff = 0; iff < patch.fields[ ik ].second.size(); ++iff )
                        {
                            ImGui::Indent(16);
                            ImGui::InputInt( util::format( "Field##field$_$", ik, iff ).c_str(), &patch.fields[ ik ].second[ iff ].first );
                            doStringImguiTextField( patch.fields[ ik ].second[ iff ].second, util::format( "Value##value$_$", ik, iff ) );
                            if ( ImGui::Button( util::format( "Remove field##remove$_$", ik, iff ).c_str() ) )
                            {
                                patch.fields[ ik ].second.erase( patch.fields[ ik ].second.begin() + iff );
                                break;
                            }
                            ImGui::Separator();
                            ImGui::Indent(-16);
                        }
                        ImGui::Indent(16);
                        if ( ImGui::Button( util::format( "Add field##field%", ik ).c_str() ) )
                        {
                            patch.fields[ ik ].second.push_back( std::pair< int, std::string >() );
                            break;
                        }
                        ImGui::Indent(-16);
                    }
                    if ( ImGui::Button( "Add key" ) )
                    {
                        patch.fields.push_back( std::pair< std::string, std::vector< std::pair< int, std::string > > >() );
                    }
                }
                //ImGui::EndChild();

                ImGui::Separator();

                ImGui::Text( "Entries" );
                showImguiHelp( "The entries in the data file you want to add, replace, or (if set to null) delete." );
                //ImGui::BeginChild( "entries" );
                {
                    for ( std::size_t ie = 0; ie < patch.entries.size(); ++ie )
                    {
                        doStringImguiTextField( patch.entries[ ie ].first, util::format( "Entry##entrykey$", ie ) );
                        doStringImguiTextField( patch.entries[ ie ].second, util::format( "Value##entryvalue$", ie ) );
                        if ( ImGui::Button( "Remove entry" ) )
                        {
                            patch.entries.erase( patch.entries.begin() + ie );
                            break;
                        }
                        ImGui::Separator();
                    }
                    if ( ImGui::Button( "Add entry" ) )
                    {
                        patch.entries.push_back( std::pair< std::string, std::string >() );
                    }
                }
                //ImGui::EndChild();
            }
        }
        ImGui::End();
    }

    if ( wizardStage == PatchWizardStage::ChooseFile )
    {
        bool open = true;
        if ( ImGui::Begin( "Patch Wizard", &open ) )
        {
            if ( !open )
                wizardStage = NotOpen;

            doStringImguiTextField( wizardFile, "(Filter)" );
            showImguiHelp( "Filter the selection box below." );

            std::vector< std::string > choicesVec;
            std::string choices;
            int sel = -1;
            for ( auto it = contentFiles.begin(); it != contentFiles.end(); ++it )
            {
                if ( it->find( wizardFile ) != std::string::npos )
                {
                    choicesVec.push_back( * it );
                    choices += ( ( * it ) + '\0' );
                    if ( ( * it ) == wizardFile )
                        sel = choicesVec.size() - 1;
                }
            }

            ImGui::Combo( "Content files", &sel, choices.c_str() );
            showImguiHelp( "The file the wizard should help you with." );
            if ( sel != -1 )
                wizardFile = choicesVec[ sel ];

            if ( ImGui::Button( "Okay" ) )
            {
                std::string stardewDir = getStardewContentsDirectory();
                util::log( "Stardew directory: $\n", stardewDir );
                if ( !wizardBase.loadFromFile( ( fs::path( stardewDir ) / ( wizardFile + ".xnb" ) ).string() ) )
                {
                    util::log( "Couldn't find content file $.\n", wizardFile );
                    tinyfd_messageBox( "Content file not found", "The content file you provided was not found.", "ok", "error", 1 );
                }
                else
                {
                    if ( xnb::Texture2DData* tex = dynamic_cast< xnb::Texture2DData* >( wizardBase.data.get() ) )
                    {
                        activeWizard.reset( new ImageCPWizard( ( * this ), wizardFile, tex ) );
                        wizardStage = PatchWizardStage::DoingWizardry;
                    }
                    else if ( xnb::DictionaryData* dict = dynamic_cast< xnb::DictionaryData* >( wizardBase.data.get() ) )
                    {
                        for ( char& c : wizardFile )
                            if ( c == '\\' )
                                c = '/';

                        if ( wizardFile.find( "Characters/Dialogue/" ) == 0 )
                        {
                            activeWizard.reset( new DialogueCPWizard( ( * this ), wizardFile, dict ) );
                            wizardStage = PatchWizardStage::DoingWizardry;
                        }
                        else
                        {
                            tinyfd_messageBox( "Can't help you there", "We can't help you patch that file.\nDo it manually using \"New Patch\".", "ok", "error", 1 );
                        }
                    }
                    else
                    {
                        tinyfd_messageBox( "Can't help you there", "We can't help you patch that file.\nDo it manually using \"New Patch\".", "ok", "error", 1 );
                    }
                }
            }
        }
        ImGui::End();
    }

    if ( activeWizard )
        activeWizard->update();

    if ( doCloseWizard )
    {
        activeWizard.reset();
        doCloseWizard = false;
    }
}

void ContentPatcherProject::eventHook( const sf::Event& event )
{
    if ( activeWizard )
        activeWizard->update( event );
}

void ContentPatcherProject::mainMenuBarHook()
{
    if ( ImGui::BeginMenu( "Patches" ) )
    {
        if ( ImGui::MenuItem( "New Patch" ) )
        {
            selPatch = patches.size();
            patches.push_back( ContentPatch() );
        }
        if ( ImGui::MenuItem( "Patch Wizard (Edit Only)", nullptr, wizardStage != NotOpen ) )
        {
            wizardStage = ChooseFile;
        }
        ImGui::Separator();

        for ( std::size_t i = 0; i < patches.size(); ++i )
        {
            const ContentPatch& patch = patches[ i ];
            if ( ImGui::MenuItem( util::format( "Patch #$ - $", i + 1, patch.name ).c_str(), nullptr, selPatch == i ) )
                selPatch = i;
        }
        ImGui::EndMenu();
    }

    if ( ImGui::BeginMenu( "Resources" ) )
    {
        if ( ImGui::MenuItem( "Refresh" ) )
        {
            refreshResourceList();
        }
        ImGui::Separator();

        for ( const auto& resource : resourceList )
        {
            ImGui::MenuItem( resource.c_str() );
        }

        ImGui::EndMenu();
    }
}

void ContentPatcherProject::drawHook( sf::RenderWindow& window )
{
    if ( activeWizard )
        activeWizard->draw( window );
}

void ContentPatcherProject::manifestHook()
{
    ImGui::Separator();
    ImGui::Text( "Config Schema" );
    for ( int ic = 0; ic < configs.size(); ++ic )
    {
        ConfigEntry& config = configs[ ic ];
        doStringImguiTextField( config.allowValues, util::format( "Allow Values##config1_$", ic ).c_str() );
        ImGui::Checkbox( util::format( "Allow Blank##configb1_$", ic ).c_str(), &config.allowBlank );
        ImGui::SameLine();
        ImGui::Checkbox( util::format( "Allow Multiple##configb2_$", ic ).c_str(), &config.allowMultiple );
        doStringImguiTextField( config.defaults, util::format( "Default##config2_$", ic ).c_str() );

        if ( ImGui::Button( util::format( "Delete config entry##configdel_$", ic ).c_str() ) )
        {
            configs.erase( configs.begin() + ic );
            break;
        }
    }
    if ( ImGui::Button( "Add config entry" ) )
        configs.push_back( ConfigEntry() );
}

void ContentPatcherProject::refreshResourceList()
{
    resourceList.clear();
    for ( fs::recursive_directory_iterator it( dir ); it != fs::recursive_directory_iterator(); ++it )
    {
        std::string ext = it->path().extension().string();
        if ( ext == ".png" || ext == ".tbin" || ext == ".xnb" )
        {
            std::string res = make_relative( fs::path( dir ), it->path() ).string();
            resourceList.push_back( res );
        }
    }
}

void ContentPatcherProject::gatherContentFiles()
{
    contentFiles.clear();
    std::string contentsDir = getStardewContentsDirectory();
    util::log( "Stardew dir: $\n", contentsDir );
    for ( fs::recursive_directory_iterator it( contentsDir ); it != fs::recursive_directory_iterator(); ++it )
    {
        if ( it->path().extension().string() == ".xnb" )
        {
            std::string path = make_relative( fs::path( contentsDir ), it->path() ).string();

            std::size_t d1 = path.find( '.' );
            if ( d1 != path.length() - 4 )
                continue; // We don't want files with languages

            util::log( "\tFound: $\n", path );
            contentFiles.push_back( path.substr( 0, path.length() - 4 ) );
        }
    }
}
