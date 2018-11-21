#include "projects/CPWizard/ImageCPWizard.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>

#include "projects/ContentPatcherProject.hpp"
#include "Program.hpp"

ImageCPWizard::ImageCPWizard( ContentPatcherProject& theProj, const std::string& theTarget, xnb::Texture2DData* theTexBase )
:   CPWizard::CPWizard( theProj ),
    target( theTarget ),
    texBase( * theTexBase )
{
    tex.loadFromImage( texBase.data[ 0 ] );
}

void ImageCPWizard::update()
{
    if ( ( draggingBg || draggingFg ) && !sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
    {
        if ( draggingFg )
            fg.setPosition( static_cast< int >( fg.getPosition().x ), static_cast< int >( fg.getPosition().y ) );
        draggingBg = false;
        draggingFg = false;
    }

    if ( ImGui::Begin( "Image Wizard" ) )
    {
        std::vector< std::string > imageList;
        std::string resources;
        int sel = -1;
        for ( const auto& res : proj.getResourceList() )
            if ( res.length() > 4 && res.substr( res.length() - 4 ) == ".png" )
            {
                imageList.push_back( res );
                resources += std::string( res.c_str() ) + '\0';

                if ( resFgStr == res )
                    sel = imageList.size() - 1;
            }

        int prevSel = sel;
        ImGui::Combo( "Image", &sel, resources.c_str() );
        if ( sel != prevSel )
        {
            resFgStr = imageList[ sel ];
            resFg.loadFromFile( ( fs::path( proj.dir ) / resFgStr ).string() );
            fg.setTexture( resFg, true );
            fg.setPosition( 0, 0 );
        }

        ImGui::Checkbox( "Overlay", &overlay );

        if ( ImGui::Button( "Crop" ) )
        {
            fg.setTexture( resFg, true );
            fg.setPosition( 0, 0 );
            cropping = true;
        }

        if ( ImGui::Button( "Apply" ) )
        {
            ContentPatcherProject::ContentPatch patch;

            patch.action = ContentPatcherProject::ContentPatch::ActionType::EditImage;
            patch.target = target;
            patch.fromFile = resFgStr;
            patch.fromArea.left = fg.getTextureRect().left;
            patch.fromArea.top = fg.getTextureRect().top;
            patch.fromArea.width = patch.toArea.width = fg.getTextureRect().width;
            patch.fromArea.height = patch.toArea.height = fg.getTextureRect().height;
            patch.toArea.left = fg.getPosition().x;
            patch.toArea.top = fg.getPosition().y;
            patch.patchMode = overlay ? ContentPatcherProject::ContentPatch::PatchMode::Overlay : ContentPatcherProject::ContentPatch::PatchMode::Replace;

            proj.addPatch( patch );
            proj.closeWizard();
        }
    }
    ImGui::End();
}

void ImageCPWizard::update( const sf::Event& event )
{
    if ( cropping && event.type == sf::Event::MouseButtonReleased )
    {
        sf::Vector2f dragTo = Program::window.mapPixelToCoords( sf::Vector2i( event.mouseButton.x, event.mouseButton.y ), myView );
        cropRect = sf::IntRect( dragFrom.x, dragFrom.y, dragTo.x - dragFrom.x, dragTo.y - dragFrom.y );
        if ( cropRect.width < 0 )
            cropRect = sf::IntRect( dragFrom.x + cropRect.width, cropRect.top, -cropRect.width, cropRect.height );
        if ( cropRect.height < 0 )
            cropRect = sf::IntRect( dragFrom.x, cropRect.top + cropRect.height, cropRect.width, -cropRect.height );

        if ( cropRect.left < 0 )
        {
            cropRect.width = cropRect.width + cropRect.left;
            cropRect.left = 0;
        }
        if ( cropRect.top < 0 )
        {
            cropRect.height = cropRect.height + cropRect.top;
            cropRect.top = 0;
        }
        if ( cropRect.left + cropRect.width >= resFg.getSize().x )
            cropRect.width = resFg.getSize().x - cropRect.left;
        if ( cropRect.top + cropRect.height >= resFg.getSize().y )
            cropRect.height = resFg.getSize().y - cropRect.top;

        fg.setTextureRect( cropRect );

        cropping = false;
    }
    if ( event.type == sf::Event::EventType::MouseButtonPressed && !ImGui::IsMouseHoveringAnyWindow() )
    {
        if ( cropping )
        {
            dragFrom = Program::window.mapPixelToCoords( sf::Vector2i( event.mouseButton.x, event.mouseButton.y ), myView );
        }
        else
        {
            if ( event.mouseButton.button == sf::Mouse::Left )
            {
                dragFrom = Program::window.mapPixelToCoords( sf::Vector2i( event.mouseButton.x, event.mouseButton.y ), myView );
                if ( fg.getGlobalBounds().contains( dragFrom ) )
                    draggingFg = true;
                else
                    draggingBg = true;
            }
        }
    }
    else if ( event.type == sf::Event::EventType::MouseWheelScrolled && !ImGui::IsMouseHoveringAnyWindow() )
    {
        sf::Vector2f worldPos = Program::window.mapPixelToCoords( sf::Vector2i( event.mouseWheelScroll.x, event.mouseWheelScroll.y ), myView );

        if ( event.mouseWheelScroll.delta < 0 )
            myView.zoom( 1.1 );
        else
            myView.zoom( 0.9 );

        sf::Vector2f newWorldPos = Program::window.mapPixelToCoords( sf::Vector2i( event.mouseWheelScroll.x, event.mouseWheelScroll.y ), myView );
        myView.move( worldPos - newWorldPos );
    }
    else if ( event.type == sf::Event::EventType::Resized )
    {
        sf::Vector2f oldCenter = myView.getCenter();
        myView = sf::View( sf::FloatRect( 0, 0, Program::window.getSize().x, Program::window.getSize().y ) );
        myView.setCenter( oldCenter );
    }
}

void ImageCPWizard::draw( sf::RenderWindow& window )
{
    if ( !initView )
    {
        myView = sf::View( sf::FloatRect( -( window.getSize().x * 1.f ) / 2.f, -( window.getSize().y * 1.f ) / 2.f, window.getSize().x, window.getSize().y ) );
        initView = true;
    }

    sf::View oldView = window.getView();
    window.setView( myView );
    {
        if ( draggingBg || draggingFg )
        {
            sf::Vector2i mouse = sf::Vector2i( sf::Mouse::getPosition( window ) );
            sf::Vector2f dragTo = Program::window.mapPixelToCoords( mouse, myView );
            if ( draggingFg )
                fg.move( dragTo - dragFrom );
            if ( draggingBg )
                myView.move( dragFrom - dragTo );
            dragFrom = Program::window.mapPixelToCoords( mouse, myView );
        }

        if ( !cropping )
        {
            sf::RectangleShape rect;
            rect.setFillColor( sf::Color::White );
            rect.setOutlineColor( sf::Color::Green );
            rect.setOutlineThickness( 3 );
            rect.setSize( sf::Vector2f( tex.getSize().x, tex.getSize().y ) );
            window.draw( rect );
            window.draw( sf::Sprite( tex ) );
        }

        sf::RectangleShape rect;
        if ( overlay )
            rect.setFillColor( sf::Color::Transparent );
        else
            rect.setFillColor( sf::Color::White );
        rect.setOutlineColor( sf::Color::Magenta );
        rect.setOutlineThickness( 3 );
        rect.setPosition( fg.getPosition() );
        if ( cropping )
        {
            rect.setPosition( dragFrom );
            auto spot = window.mapPixelToCoords( sf::Mouse::getPosition( window ), myView );
            rect.setSize( sf::Vector2f( spot.x - dragFrom.x, spot.y - dragFrom.y ) );
        }
        else
            rect.setSize( sf::Vector2f( fg.getTextureRect().width, fg.getTextureRect().height ) );
        if ( !cropping || sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
            window.draw( rect );
        window.draw( fg );
        if ( cropping && sf::Mouse::isButtonPressed( sf::Mouse::Left ) )
            window.draw( rect );
    }
    window.setView( oldView );
}
