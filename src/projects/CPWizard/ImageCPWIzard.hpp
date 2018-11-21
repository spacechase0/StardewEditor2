#ifndef IMAGECPWIZARD_HPP
#define IMAGECPWIZARD_HPP

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <xnb/TextureType.hpp>

#include "projects/CPWizard/CPWizard.hpp"

class ImageCPWizard : public CPWizard
{
    public:
        ImageCPWizard( ContentPatcherProject& theProj, const std::string& theTarget, xnb::Texture2DData* theTexBase );

        virtual void update() override;
        virtual void update( const sf::Event& event ) override;
        virtual void draw( sf::RenderWindow& window ) override;

    private:
        const std::string target;
        xnb::Texture2DData& texBase;
        sf::Texture tex;

        std::string resFgStr;
        sf::Texture resFg;
        sf::Sprite fg;
        bool overlay = true;

        sf::View myView;
        bool initView = false;

        bool cropping = false;
        sf::IntRect cropRect;

        bool draggingBg = false;
        bool draggingFg = false;
        sf::Vector2f dragFrom;
};

#endif // IMAGECPWIZARD_HPP
