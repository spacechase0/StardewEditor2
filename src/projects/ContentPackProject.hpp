#ifndef CONTENTPACKPROJECT_HPP
#define CONTENTPACKPROJECT_HPP

#include "Project.hpp"

class ContentPackProject : public Project
{
    public:
        virtual void save() override;

        const std::string dir;

    protected:
        ContentPackProject( const std::string cpFor, const std::string& theDir );
        ContentPackProject( json theManifest, const std::string& theDir );

        virtual void updateHook() override;

        virtual void manifestHook();

        json manifest;
};

#endif // CONTENTPACKPROJECT_HPP
