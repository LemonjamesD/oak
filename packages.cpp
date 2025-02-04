/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "packages.hpp"
#include "tags.hpp"

/*
File oak_package_info.txt:

NAME = "name goes here"
VERSION = "0.0.1"
LICENSE = "GPLv3"
DATE = "April 1st, 2003"
SOURCE = "www.google.com"
PATH = ""
AUTHOR = "John Smith"
EMAIL = "JSmith@gmail.com"
ABOUT = "A demo package"
INCLUDE = "main_include.oak"
SYS_DEPS = ""
*/

// Macros that may conflict with other files; Thus, included only in the body
#define sm_system(command, message)                                                                                    \
    {                                                                                                                  \
        printf("%s\n", string(command).c_str());                                                                       \
        system(string(command).c_str()) == 0 ? 0 : throw package_error(message " (System call " #command " failed.)"); \
    }

map<string, packageInfo> packages;

ostream &operator<<(ostream &strm, const packageInfo &info)
{
    strm << "Package '" << info.name << "'\n"
         << "Version '" << info.version << "'\n"
         << "License '" << info.license << "'\n"
         << "Released '" << info.date << "'\n"
         << "Author(s) '" << info.author << "'\n"
         << "Email(s) '" << info.email << "'\n"
         << "Via '" << info.source << ":/" << info.path << "'\n\n"
         << info.description << "\n\n"
         << "Include path '/usr/include/oak/" << info.name << "/" << info.toInclude << "'\n"
         << "System Deps: '" << info.sysDeps << "'\n";

    return strm;
}

void install(const string &What)
{
    static string installCommand = "";
    string line;

    if (installCommand == "")
    {
        // Get os-release info
        system("cat /etc/os-release | grep ^ID= > .oak_build/temp.txt");

        ifstream osName(".oak_build/temp.txt");
        pm_assert(osName.is_open(), "Failed to poll OS name");
        getline(osName, line);
        osName.close();

        if (line.substr(0, 3) == "ID=")
        {
            line.erase(0, 3);
        }

        for (int i = 0; i < line.size(); i++)
        {
            line[i] = tolower(line[i]);
        }

        if (line == "arch")
        {
            installCommand = "sudo pacman --needed -S ";
        }
        else if (line == "ubuntu")
        {
            installCommand = "sudo apt-get install ";
        }
        else if (line == "fedora")
        {
            installCommand = "sudo dnf install ";
        }
    }

    int result = -1;
    while (result != 0)
    {
        if (installCommand == "")
        {
            cout << tags::yellow_bold
                 << "Local Linux installation command could not be automatically detected\n"
                    "for distribution '"
                 << line << "' during installation of packages '" << What
                 << "'\n"
                    "Please enter a command prefix which will install all packages\n"
                    "which follow it (for example, in Ubuntu: `sudo apt-get install`).\n"
                    "To manually install these instead, enter 'MANUAL'.\n"
                 << "Command: " << tags::reset;

            getline(cin, installCommand);
            if (installCommand == "MANUAL")
            {
                cout << tags::yellow_bold << "Skipping automatic installation. Be sure to manually install\n"
                     << "package(s) '" << What << "' or things may break!\n"
                     << tags::reset << flush;
                break;
            }
        }

        string command = installCommand + " " + What;
        cout << command << '\n';

        result = system(command.c_str());

        if (result != 0)
        {
            cout << tags::red_bold << "Command `" << command << "` failed. Please check your install command.\n"
                 << "Note: If this is persistant, enter 'MANUAL' to skip this step. However, be\n"
                 << "sure to install the requested package(s) ('" << What << "') manually.\n"
                 << tags::reset;
            installCommand = "";
        }
    }

    return;
}

// Remove any leading or trailing quotes
void cleanString(string &What)
{
    while (What.front() == '"' || What.front() == '\'')
    {
        What.erase(What.begin());
    }

    while (What.back() == '"' || What.back() == '\'')
    {
        What.pop_back();
    }

    return;
}

packageInfo loadPackageInfo(const string &Filepath)
{
    ifstream inp(Filepath);
    pm_assert(inp.is_open(), "Failed to load file '" + Filepath + "'");

    packageInfo toAdd;

    string name, content, garbage;
    while (!inp.eof())
    {
        inp >> name;
        if (inp.eof())
        {
            break;
        }

        // Commenting
        if (name.size() >= 2 && name.substr(0, 2) == "//")
        {
            getline(inp, garbage);
            continue;
        }

        inp >> garbage;
        pm_assert(garbage == "=", "Malformed package info file.");
        if (inp.eof())
        {
            break;
        }

        inp >> content;

        if (content[0] == '"' || content[0] == '\'')
        {
            while (content.back() != content.front())
            {
                inp >> garbage;
                content += " " + garbage;

                if (inp.eof())
                {
                    break;
                }
            }

            content = content.substr(1, content.size() - 2);
        }

        cleanString(content);

        // Adds case resiliency
        for (int i = 0; i < name.size(); i++)
        {
            name[i] = toupper(name[i]);
        }

        if (name == "NAME")
        {
            toAdd.name = content;
        }
        else if (name == "VERSION")
        {
            toAdd.version = content;
        }
        else if (name == "LICENSE")
        {
            toAdd.license = content;
        }
        else if (name == "SOURCE")
        {
            toAdd.source = content;
        }
        else if (name == "AUTHOR")
        {
            toAdd.author = content;
        }
        else if (name == "EMAIL")
        {
            toAdd.email = content;
        }
        else if (name == "ABOUT")
        {
            toAdd.description = content;
        }
        else if (name == "DATE")
        {
            toAdd.date = content;
        }
        else if (name == "INCLUDE")
        {
            toAdd.toInclude = content;
        }
        else if (name == "SYS_DEPS")
        {
            toAdd.sysDeps = content;
        }
        else if (name == "PATH")
        {
            toAdd.path = content;
        }
        else
        {
            throw package_error("Invalid item '" + name + "'");
        }
    }

    inp.close();

    packages[toAdd.name] = toAdd;
    return toAdd;
}

void savePackageInfo(const packageInfo &Info, const string &Filepath)
{
    ofstream out(Filepath);
    pm_assert(out.is_open(), "Failed to open file '" + Filepath + "'");

    out << "NAME = '" << Info.name << "'\n"
        << "VERSION = '" << Info.version << "'\n"
        << "LICENSE = '" << Info.license << "'\n"
        << "DATE = '" << Info.date << "'\n"
        << "AUTHOR = '" << Info.author << "'\n"
        << "EMAIL = '" << Info.email << "'\n"
        << "SOURCE = '" << Info.source << "'\n"
        << "ABOUT = '" << Info.description << "'\n"
        << "INCLUDE = '" << Info.toInclude << "'\n"
        << "SYS_DEPS = '" << Info.sysDeps << "'\n"
        << "PATH = '" << Info.path << "'\n";

    out.close();
    return;
}

/*
Imagine using a compiled language for scripting; Couldn't be me
*/
void downloadPackage(const string &URLArg, const bool &Reinstall, const string &Path)
{
    string URL = URLArg;

    // Check if package is already installed
    for (auto p : packages)
    {
        if (p.second.source == URLArg || p.second.name == URLArg)
        {
            if (Reinstall)
            {
                URL = p.second.source;
            }
            else
            {
                cout << tags::yellow_bold << "Package '" << p.first << "' was already installed.\n" << tags::reset;
                return;
            }
        }
    }

    // Create temp location
    string tempFolderName = PACKAGE_TEMP_LOCATION "oak_tmp_" + to_string(time(NULL));

    sm_system("mkdir -p " PACKAGE_TEMP_LOCATION,
              "Failed to create packages directory. Ensure you have sufficient privileges.");

    // Clone package using git
    try
    {
        // URL
        if (system((string(CLONE_COMMAND) + URL + " " + tempFolderName + " > /dev/null").c_str()) != 0)
        {
            throw package_error("Git resolution failure; Likely an invalid source.");
        }
    }
    catch (package_error &e)
    {
        cout << tags::yellow_bold << e.what() << '\n' << "Attempting git-less copy...\n" << tags::reset;

        try
        {
            // Local file
            sm_system("cp -r " + URL + " " + tempFolderName, "Local copy failed; Is neither URL nor filepath.");
        }
        catch (package_error &e)
        {
            // Package from packages_list.txt

            cout << "Checking /usr/include/oak/packages_list.txt...\n";

            // Load packages_list.txt
            ifstream packagesList(PACKAGES_LIST_PATH);
            pm_assert(packagesList.is_open(),
                      URL + " is not a valid package URL or name and failed to load packages list.");

            string line;
            while (getline(packagesList, line))
            {
                if (line.size() >= 2 && line.substr(0, 2) == "//")
                {
                    continue;
                }
                else
                {
                    pm_assert(line.find(' ') != string::npos, "Malformed packages_list.txt");

                    cout << 330 << '\t' << line << '\n';

                    // name source path
                    string name, source, path;

                    auto breakPoint = line.find(' ');

                    name = line.substr(0, breakPoint);
                    source = line.substr(breakPoint + 1);

                    cout << 340 << '\t' << name << '\t' << source << '\n';

                    pm_assert(source.find(' ') != string::npos, "Malformed packages_list.txt");
                    breakPoint = source.find(' ');

                    path = source.substr(breakPoint + 1);
                    source = source.substr(0, breakPoint);

                    cout << 347 << '\t' << name << '\t' << source << '\t' << path << '\n';

                    if (name == URL)
                    {
                        cout << "Package '" << name << "' found in /usr/include/oak/packages_list.txt w/ repo URL of '"
                             << source << "'\n";

                        downloadPackage(source, Reinstall, path);
                        return;
                    }
                }
            }

            // If it has gotten to this point, the following is true:
            // - It is not present in packages_list.txt
            // - It is not a valid URL
            // - It is not a valid local file
            // So it should error rather than proceed.

            cout << tags::red_bold << "\nPackage '" << URLArg << "'\n"
                 << "does not exist locally, is not a valid Git repo, and\n"
                 << "does not have an installation URL known by acorn.\n"
                 << tags::reset;
            throw package_error("Package '" + URLArg + "' does not exist in any form.");
        }
    }

    try
    {
        string path = Path;
        if (path == "")
        {
            path = ".";
        }

        while (system(("[ -f " + tempFolderName + "/" + path + "/" + INFO_FILE " ]").c_str()) != 0)
        {
            cout << tags::yellow_bold << "Failed to locate info file.\n"
                 << "Enter the path within the folder to install from [default .]: " << tags::reset;
            getline(cin, path);
            if (path == "")
            {
                path = ".";
            }

            pm_assert(path.find("..") == string::npos, "Illegal path: May not contain '..'");
        }

        // At this point, info file must exist

        bool needsMake = false;
        needsMake = (system(("[ -f " + tempFolderName + "/" + path + "/Makefile ]").c_str()) == 0 ||
                     system(("[ -f " + tempFolderName + "/" + path + "/makefile ]").c_str()) == 0);

        // Read info file
        packageInfo info = loadPackageInfo(tempFolderName + "/" + path + "/" + INFO_FILE);

        cout << tags::green << "Loaded package from " << URL << "\n" << info << '\n' << tags::reset;

        // Ensure valid formatting
        for (char c : info.name)
        {
            pm_assert(!('A' <= c && c <= 'Z'), "Cannot install package with illegal name.");
        }

        // Prepare destination
        string destFolderName = PACKAGE_INCLUDE_PATH + info.name;
        sm_system("sudo rm -rf " + destFolderName, "Failed to clear old package files.");
        sm_system("sudo mkdir -p " + destFolderName,
                  "Failed to create package folder in /usr/include/oak; Check user permissions.");

        // Install system deps
        if (info.sysDeps != "")
        {
            install(info.sysDeps);
        }

        // Make package if needed
        if (needsMake)
        {
            if (system(("make -C " + tempFolderName + "/" + path).c_str()) != 0)
            {
                throw package_error("Make failure; See cout for details.");
            }
        }

        // Copy files
        system(("sudo cp " + tempFolderName + "/" + path + "/*.c " + destFolderName).c_str());
        system(("sudo cp " + tempFolderName + "/" + path + "/*.h " + destFolderName).c_str());
        system(("sudo cp " + tempFolderName + "/" + path + "/*.o " + destFolderName).c_str());
        system(("sudo cp " + tempFolderName + "/" + path + "/*.oak " + destFolderName).c_str());
        system(("sudo cp " + tempFolderName + "/" + path + "/*.txt " + destFolderName).c_str());

        // Clean up garbage; Doesn't really matter if this fails
        cout << "sudo rm -rf " PACKAGE_TEMP_LOCATION << '\n';
        if (system("sudo rm -rf " PACKAGE_TEMP_LOCATION) != 0)
        {
            cout << tags::yellow_bold << "Warning: Failed to erase trash folder '" << PACKAGE_TEMP_LOCATION << "'.\n"
                 << "If left unfixed, this could cause issues.\n"
                 << tags::reset;
        }
    }
    catch (runtime_error &e)
    {
        // Clean up garbage

        if (system("sudo rm -rf " PACKAGE_TEMP_LOCATION) != 0)
        {
            cout << tags::yellow_bold << "Warning: Failed to erase trash folder '" << PACKAGE_TEMP_LOCATION << "'.\n"
                 << "If left unfixed, this could cause issues.\n"
                 << tags::reset;
        }

        throw package_error(e.what());
    }

    return;
}

vector<string> getPackageFiles(const string &Name)
{
    // If package is not already loaded
    if (packages.count(Name) == 0)
    {
        if (system(("[ -d /usr/include/oak/" + Name + " ]").c_str()) == 0)
        {
            // Installed, but not loaded; Load and continue
            loadPackageInfo("/usr/include/oak/" + Name + "/" + INFO_FILE);
        }
        else
        {
            // Not installed or loaded; Throw error
            throw package_error("Package '" + Name + "' could not be found. Ensure it is installed.");
        }
    }

    // Loaded and installed
    packageInfo info = packages[Name];
    vector<string> out;
    string cur, toSplit = info.toInclude;

    for (int i = 0; i < toSplit.size(); i++)
    {
        if (toSplit[i] == ',')
        {
            if (cur != "")
            {
                out.push_back(PACKAGE_INCLUDE_PATH + Name + "/" + cur);
            }
            cur = "";
        }
        else
        {
            cur.push_back(toSplit[i]);
        }
    }

    if (cur != "")
    {
        out.push_back(PACKAGE_INCLUDE_PATH + Name + "/" + cur);
    }

    return out;
}
