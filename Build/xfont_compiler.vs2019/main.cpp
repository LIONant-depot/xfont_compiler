
#include "../../src/xfont_compiler.h"

//---------------------------------------------------------------------------------

struct font_pipeline_compiler : xresource_pipeline::compiler::base
{
    static constexpr xcore::guid::rcfull<> full_guid_v
    { .m_Type = xcore::guid::rctype<>         { "resource.pipeline", "plugin" }
    , .m_Instance = xcore::guid::rcinstance<> { "xfont" }
    };

    virtual xcore::guid::rcfull<> getResourcePipelineFullGuid() const noexcept override
    {
        return full_guid_v;
    }

    virtual xcore::err onCompile(void) noexcept override
    {
        if (auto Err = xfont_compiler::descriptor::Serialize(m_CompilerOptions, m_ResourceDescriptorPathFile.data(), true); Err)
            return Err;

        m_Compiler->LoadFont(m_AssetsRootPath, m_CompilerOptions);
        m_Compiler->Compile(m_CompilerOptions
            , xresource_pipeline::compiler::base::m_OptimizationType
        );

        for (auto& T : m_Target)
        {
            if (T.m_bValid) m_Compiler->Serialize(T.m_DataPath.data());
        }

        return {};
    }

    xfont_compiler::descriptor                  m_CompilerOptions{};
    std::unique_ptr<xfont_compiler::instance>   m_Compiler = xfont_compiler::MakeInstance();
};

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------

int main( int argc, const char* argv[] )
{
    xcore::Init("xfont_compiler");

    auto FontCompilerPipeline = std::make_unique<font_pipeline_compiler>();

    if constexpr (false)
    {
        xresource_pipeline::config::info Info
        { .m_RootAssetsPath = "x64/Assets"
        };
        Info.m_ResourceTypes.push_back
        (xresource_pipeline::config::resource_type
            { .m_FullGuid                = font_pipeline_compiler::full_guid_v
            , .m_ResourceTypeName        = "xfont"
            , .m_bDefaultSettingInEditor = true
            });

        (void)xresource_pipeline::config::Serialize(Info, "ResourcePipeline.config", false);
    }

    if constexpr (false)
    {
        xfont_compiler::descriptor Option;
        Option.m_Source.m_Font.m_SourcePath = xcore::string::Fmt("FinalV1.jpg");
        (void)xfont_compiler::descriptor::Serialize(Option, "ResourceDesc.txt", false);
    }

    //
    // Parse parameters
    //
    if( auto Err = FontCompilerPipeline->Parse( argc, argv ); Err )
    {
        printf( "%s\nERROR: Fail to compile\n", Err.getCode().m_pString );
        return -1;
    }

    //
    // Start compilation
    //
    if( auto Err = FontCompilerPipeline->Compile(); Err )
    {
        printf("%s\nERROR: Fail to compile(2)\n", Err.getCode().m_pString);
        return -1;
    }


    return 0;
}