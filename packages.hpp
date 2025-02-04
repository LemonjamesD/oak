/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Package loading for Oak.
*/

#ifndef PACKAGES_HPP
#define PACKAGES_HPP

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "tags.hpp"

using namespace std;

#define PACKAGE_TEMP_LOCATION "/tmp/oak_packages/"
#define PACKAGE_INCLUDE_PATH "/usr/include/oak/"
#define INFO_FILE "oak_package_info.txt"

#define PACKAGES_LIST_PATH "/usr/include/oak/packages_list.txt"

#define CLONE_COMMAND "git clone "

struct packageInfo
{
    string name;        // Package name
    string version;     // Package version
    string license;     // Package license
    string date;        // Date the current version was released
    string author;      // Self-explanitory
    string email;       // See maintainer name
    string source;      // URL package was downloaded from
    string path;        // Path from URL to get to the install point
    string description; // Package description

    string toInclude; // File within /usr/include/oak/$(PACKAGE_NAME) to include!();

    string sysDeps;
};

extern string installCommand;
void install(const string &What);

ostream &operator<<(ostream &strm, const packageInfo &info);

class package_error : public runtime_error
{
  public:
    package_error(const string &What) : runtime_error(What)
    {
    }
};

#define pm_assert(expression, message)                                                                                 \
    ((bool)(expression) ? true : throw package_error(message " (Failed assertion: '" #expression "')"))

extern map<string, packageInfo> packages;

// Loads a package info file
packageInfo loadPackageInfo(const string &Filepath);

void loadAllPackages();

// Saves a package info file
void savePackageInfo(const packageInfo &Info, const string &Filepath);

// Downloads a package from a URL via git
void downloadPackage(const string &URL, const bool &Reinstall = false, const string &Path = "");

// Get the include!() -ed files of a package given name and possibly URL
vector<string> getPackageFiles(const string &Name);

#endif
