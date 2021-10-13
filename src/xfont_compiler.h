#ifndef XFONT_COMPILER_H
#define XFONT_COMPILER_H
#pragma once

#include "xresource_pipeline.h"

namespace xfont_compiler
{
    static constexpr xcore::guid::rcfull<> full_guid_v
    { .m_Type = xcore::guid::rctype<>         { "resource.pipeline", "plugin" }
    , .m_Instance = xcore::guid::rcinstance<> { "xfont" }
    };
}

#include "xfont_compiler_descriptor.h"
#include "xfont_compiler_instance.h"

#endif