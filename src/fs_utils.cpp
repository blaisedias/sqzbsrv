/*

Copyright (C) 2014,2015  Blaise Dias

This file is part of sqzbsrv.

sqzbsrv is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

sqzbsrv is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with sqzbsrv.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <vector>
#include <algorithm>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "fs_utils.h"

//
// g++ -o fs_utils.cpp  -lboost_filesystem -lboost_system
// g++ -c fs_utils.cpp
//
//
namespace boostfs = boost::filesystem;

namespace fs_utils {

bool verbose = false;    

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
                if (verbose)
                    std::cout << std::string(dir_iter->path().string()) << std::endl;
                continue;
            }
            
            if (boostfs::is_directory(dir_iter->status()))
            {
                // if (handle_directory(dir_iter->path().string().c_str()))
                //    dirwalk(dir_iter->path().string().c_str(), handle_file, handle_directory);
                directories.push_back(std::string(dir_iter->path().string()));
                if (verbose)
                    std::cout << std::string(dir_iter->path().string()) << std::endl;
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

uintmax_t file_size(const std::string& filename)
{
    return boost::filesystem::file_size(filename);
}

uintmax_t file_size(const char* f)
{
    return file_size(std::string(f));
}


time_t file_timestamp(const std::string& filename)
{
    return boost::filesystem::last_write_time(filename);
}

time_t file_timestamp(const char* f)
{
    return file_timestamp(std::string(f));
}

void path_append(std::string& path, const char *frag)
{
    //FIXME: assuming unix like paths and separators
    //FIXME: utf8
    static  const char *sep = "/";
    const char* const cpath = path.c_str();
    size_t len = strlen(cpath);
    const char* p = cpath;
    p += strlen(cpath) - 1;
    if (!(len && *p == *sep))
        path.append(sep);
    p = frag;
    // check for a single separator at frag start, a sqqeunce of more than 1 separators 
    // is meaningless and we cannot adjust for that.
    if (*p == *sep)
        p++;
    path.append(p);
}

void path_append(std::string& path, const std::string& frag)
{
    path_append(path, frag.c_str());
}

std::string dirname(std::string& filepath)
{
    return boost::filesystem::path(filepath).parent_path().generic_string();
}

std::string dirname(const char* filepath)
{
    return boost::filesystem::path(filepath).parent_path().generic_string();
}


std::string filename(std::string& filepath)
{
    return boost::filesystem::path(filepath).filename().generic_string();
}

std::string filename(const char* filepath)
{
    return boost::filesystem::path(filepath).filename().generic_string();
}

bool exists(const char* filepath)
{
    return boost::filesystem::exists(filepath);
}

}
