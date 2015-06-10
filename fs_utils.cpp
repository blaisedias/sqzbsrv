#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "fs_utils.h"
#include <iostream>
#include <vector>
#include <algorithm>

//
// g++ -o fs_utils.cpp  -lboost_filesystem -lboost_system
// g++ -c fs_utils.cpp
//
//
namespace boostfs = boost::filesystem;

namespace fs_utils {
void dirwalk(const char * path_str,
        int (*handle_file)(const char *),
        int (*handle_directory)(const char *),
        bool ignore_sym_links)
{
    boostfs::path full_path(boostfs::initial_path<boostfs::path>());
    full_path = boostfs::system_complete(boostfs::path( path_str));
    if(!boostfs::exists(full_path))
        return;

    if (!boostfs::is_directory(full_path))
        return;

    boostfs::directory_iterator dir_iter_end;

    std::vector<std::string> files;
    std::vector<std::string> directories;
    for(boostfs::directory_iterator dir_iter(full_path); dir_iter != dir_iter_end; ++dir_iter)
    {
        try
        {
            if (boostfs::is_symlink(dir_iter->symlink_status()))
            {
                if (ignore_sym_links)
                    continue;
            }

            if (boostfs::is_regular(dir_iter->status()))
            {
                //handle_file(dir_iter->path().string().c_str());
                files.push_back(std::string(dir_iter->path().string()));
                continue;
            }
            
            if (boostfs::is_directory(dir_iter->status()))
            {
                // if (handle_directory(dir_iter->path().string().c_str()))
                //    dirwalk(dir_iter->path().string().c_str(), handle_file, handle_directory);
                directories.push_back(std::string(dir_iter->path().string()));
                continue;
            }
        }
        catch (const std::exception &ex)
        {
            std::cout << dir_iter->path().leaf() << " " << ex.what() << std::endl;
        }
    }

    std::sort(files.begin(), files.end());
    for(unsigned indx=0; indx < files.size(); indx++)
    {
        handle_file(files[indx].c_str());
    }
    sort(directories.begin(), directories.end());
    for(unsigned indx=0; indx < directories.size(); indx++)
    {
        if (handle_directory(directories[indx].c_str()))
            dirwalk(directories[indx].c_str(), handle_file, handle_directory);
    }
}


void handler::dirwalk(const char * path_str, bool ignore_sym_links)
{
    boostfs::path full_path(boostfs::initial_path<boostfs::path>());
    full_path = boostfs::system_complete(boostfs::path( path_str));
    if(!boostfs::exists(full_path))
        return;

    if (!boostfs::is_directory(full_path))
        return;

    boostfs::directory_iterator dir_iter_end;

    std::vector<std::string> files;
    std::vector<std::string> directories;
    for(boostfs::directory_iterator dir_iter(full_path); dir_iter != dir_iter_end; ++dir_iter)
    {
        try
        {
            if (boostfs::is_symlink(dir_iter->symlink_status()))
            {
                if (ignore_sym_links)
                    continue;
            }

            if (boostfs::is_regular(dir_iter->status()))
            {
                //handle_file(dir_iter->path().string().c_str());
                files.push_back(std::string(dir_iter->path().string()));
                continue;
            }
            
            if (boostfs::is_directory(dir_iter->status()))
            {
                // if (handle_directory(dir_iter->path().string().c_str()))
                //    dirwalk(dir_iter->path().string().c_str(), handle_file, handle_directory);
                directories.push_back(std::string(dir_iter->path().string()));
                continue;
            }
        }
        catch (const std::exception &ex)
        {
            std::cout << dir_iter->path().leaf() << " " << ex.what() << std::endl;
        }
    }

    std::sort(files.begin(), files.end());
    for(unsigned indx=0; indx < files.size(); indx++)
    {
        handle_file(files[indx].c_str());
    }
    sort(directories.begin(), directories.end());
    for(unsigned indx=0; indx < directories.size(); indx++)
    {
        if (handle_directory(directories[indx].c_str()))
            dirwalk(directories[indx].c_str());
    }
}

}