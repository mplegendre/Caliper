// Copyright (c) 2017, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory.
//
// This file is part of Caliper.
// Written by David Boehme, boehme3@llnl.gov.
// LLNL-CODE-678900
// All rights reserved.
//
// For details, see https://github.com/scalability-llnl/Caliper.
// Please also see the LICENSE file for our additional BSD notice.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/// \file format_util.h
/// Helper functions to write strings.

#include "format_util.h"

namespace
{

const char whitespace[80+1] =
    "                                        "
    "                                        ";

}

std::ostream&
util::pad_right(std::ostream& os, const std::string& str, std::size_t width)
{
    os << str;

    if (str.size() > width)
        os << ' ';
    else {
        std::size_t s = 1 + width - str.size();

        for ( ; s > 80; s -= 80)
            os << ::whitespace;

        os << ::whitespace+(80-s);
    }
    
    return os;
}

std::ostream&
util::pad_left (std::ostream& os, const std::string& str, std::size_t width)
{
    if (str.size() < width) {
        std::size_t s = width - str.size();

        for ( ; s > 80; s -= 80)
            os << ::whitespace;

        os << ::whitespace+(80-s);
    }

    os << str << ' ';
    
    return os;
}

std::string
util::clamp_string(const std::string& str, std::size_t max_width)
{
    if (str.length() < max_width)
        return str;
    if (max_width < 4)
        return str.substr(0, max_width);

    std::string ret;
    ret.reserve(1 + max_width);
    
    ret.append(str, 0, max_width/2-1);
    ret.append("~~");
    ret.append(str, str.length()-(max_width/2-1), std::string::npos);

    return ret;
}
