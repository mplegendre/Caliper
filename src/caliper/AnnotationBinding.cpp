// Copyright (c) 2015-2017, Lawrence Livermore National Security, LLC.
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

// AnnotationBinding.cpp
// Base class for implementing Caliper-to-X annotation bindings

#include "caliper/AnnotationBinding.h"

#include "caliper/common/Node.h"
#include "caliper/common/Log.h"

#include "caliper/common/filters/RegexFilter.h"

using namespace cali;

//
// --- Static data
//

const cali::ConfigSet::Entry AnnotationBinding::s_configdata[] = {
    { "regex_filter", CALI_TYPE_STRING, "",
      "Regular expression for matching annotations",
      "Regular expression for matching annotations"
    },
    { "trigger_attributes", CALI_TYPE_STRING, "",
      "List of attributes that trigger the annotation binding",
      "List of attributes that trigger the annotation binding"
    },
    { "", CALI_TYPE_BOOL, "true",
      "Whether the condition of the filter says what to include or what to exclude",
      "Whether the condition of the filter says what to include or what to exclude"
    },
    cali::ConfigSet::Terminator
};

namespace
{

bool has_marker(const Attribute& attr, const Attribute& marker_attr)
{
    cali_id_t marker_attr_id = marker_attr.id();
    
    for (const Node* node = attr.node()->first_child(); node; node = node->next_sibling())
        if (node->attribute() == marker_attr_id)
            return true;

    return false;
}

}

//
// --- Implementation
//

AnnotationBinding::~AnnotationBinding()
{
    delete m_filter;
}

void
AnnotationBinding::check_attribute(Caliper* c, Channel* chn, const Attribute& attr)
{
    int prop = attr.properties();

    if (m_trigger_attr_names.empty()) {
        // By default, enable binding only for class.nested attributes
        if (!(prop & CALI_ATTR_NESTED))
            return;
    } else {
        if (std::find(m_trigger_attr_names.begin(), m_trigger_attr_names.end(),
                      attr.name()) == m_trigger_attr_names.end())
            return;
    }
    
    // Add the binding marker for this attribute
    
    Variant v_true(true);
    c->make_tree_entry(m_marker_attr, v_true, c->node(attr.node()->id()));
    
    // Invoke derived functions

    on_mark_attribute(c, chn, attr);
    
    Log(2).stream() << "Adding " << this->service_tag()
                    << " bindings for attribute \"" << attr.name()
                    << "\" in " << chn->name() << " channel" << std::endl;
}

void
AnnotationBinding::create_attr_cb(Caliper*         c,
                                  Channel*      chn,
                                  const Attribute& attr)
{
    check_attribute(c, chn, attr);
}

void
AnnotationBinding::begin_cb(Caliper* c, Channel* chn, const Attribute& attr, const Variant& value)
{
    if (attr.skip_events())
        return;
    if (!::has_marker(attr, m_marker_attr))
        return;
    if (m_filter && !m_filter->filter(attr, value))
        return;

    this->on_begin(c, chn, attr, value);
}

void
AnnotationBinding::end_cb(Caliper* c, Channel* chn, const Attribute& attr, const Variant& value)
{
    if (attr.skip_events())
        return;
    if (!::has_marker(attr, m_marker_attr))
        return;
    if (m_filter && !m_filter->filter(attr, value))
        return;

    this->on_end(c, chn, attr, value);
}

void
AnnotationBinding::base_pre_initialize(Caliper* c, Channel* chn)
{
    const char* tag = service_tag();

    m_config = chn->config().init(tag, s_configdata);

    if (m_config.get("regex").to_string().size() > 0)
        m_filter = new RegexFilter(tag, m_config);

    m_trigger_attr_names = 
        m_config.get("trigger_attributes").to_stringlist(",:");

    std::string marker_attr_name("cali.binding.");
    marker_attr_name.append(tag);
    marker_attr_name.append("#");
    marker_attr_name.append(std::to_string(chn->id()));

    m_marker_attr =
        c->create_attribute(marker_attr_name, CALI_TYPE_BOOL,
                            CALI_ATTR_HIDDEN | CALI_ATTR_SKIP_EVENTS);
}

void
AnnotationBinding::base_post_initialize(Caliper* c, Channel* chn)
{
    // check and mark existing attributes
    
    std::vector<Attribute> attributes = c->get_all_attributes();
    
    for (const Attribute& attr : attributes)
        check_attribute(c, chn, attr);
}
