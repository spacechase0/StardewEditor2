#ifndef UTIL_HPP
#define UTIL_HPP

#include <boost/filesystem.hpp>

std::string getStardewContentsDirectory();

void doStringImguiTextField(std::string& str, const std::string& displayName);

void showImguiHelp( const std::string& str );

boost::filesystem::path make_relative( boost::filesystem::path a_From, fs::path a_To );

#ifdef SFML_SYSTEM_WINDOWS
std::string getRegistryValue( const std::string& key, const std::string& value );
#endif

#endif // UTIL_HPP
