namespace xfont_compiler
{
    struct descriptor : xresource_pipeline::descriptor::base
    {
        using parent = xresource_pipeline::descriptor::base;

        struct font
        {
            xcore::cstring  m_SourcePath         {};
            xcore::cstring  m_UnicodeTxtFilePath {};
            int             m_Size               { 32 };
        };

        struct clip_art
        {
            xcore::cstring  m_SourcePath         {};
            int             m_UnicodeChar        {}; 
            int             m_Size               { 64 };
        };

        struct source
        {
            font                    m_Font      {};
            xcore::vector<clip_art> m_ClipArt   {};
        };

        inline static constexpr auto type_v = std::array
        { xcore::string::constant("MSDF")        // will save as R8G8B8U8
        , xcore::string::constant("MTSDF")       // will save as R8G8B8A8
        , xcore::string::constant("PSDF")        // will save as R8
        , xcore::string::constant("SDF")         // will save as R8
        };

        enum class type : std::uint8_t
        { MSDF
        , MTSDF
        , PSDF
        , SDF
        , SOFT_MASK
        , HARD_MASK
        , ENUM_COUNT
        };

        struct generation
        {
            int             m_BorderSize    { 0 };
            int             m_BLurSize      { 2 };
            type            m_Type          { type::SDF };
        };

        descriptor()
        {
            parent::m_Version.m_Major = 1;
            parent::m_Version.m_Minor = 0;
        }

        static inline xcore::err Serialize(descriptor& Options, std::string_view FilePath, bool isRead) noexcept;

        source      m_Source;
        generation  m_Generation;
    };

    //-------------------------------------------------------------------------------------------------------

    xcore::err descriptor::Serialize( descriptor& Options, std::string_view FilePath, bool isRead ) noexcept
    {
        xcore::textfile::stream Stream;
        xcore::err              Error;

        // Open file
        if( auto Err = Stream.Open(isRead, FilePath, xcore::textfile::file_type::TEXT, xcore::textfile::flags{ .m_isWriteFloats = true, .m_isWriteEndianSwap = false } ); Err )
            return Err;

        if( auto Err = Options.parent::Serialize( Stream, isRead ); Err )
            return Err;

        if (Stream.Record(Error, "Sources"
            , [&](std::size_t, xcore::err& Err)
            {   
                int  nClips = static_cast<int>(Options.m_Source.m_ClipArt.size());
                0
                || (Err = Stream.Field("nClips",                nClips))
                || (Err = Stream.Field("FontPath",              Options.m_Source.m_Font.m_SourcePath  ))
                || (Err = Stream.Field("FontUnicodeTextPath",   Options.m_Source.m_Font.m_UnicodeTxtFilePath  ))
                || (Err = Stream.Field("FontSize",              Options.m_Source.m_Font.m_Size  ))
                ;

                if( false == isRead )
                {
                    Options.m_Source.m_ClipArt.appendList(nClips);
                }

            })) return Error;

        if( Stream.Record( Error, "ClipArtFiles"
            , [&]( std::size_t& Count, xcore::err& Err )
            {
                if( isRead ) xassert( Count == Options.m_Source.m_ClipArt.size());
                else         Count = Options.m_Source.m_ClipArt.size();
            }
            , [&]( std::size_t I, xcore::err& Err )
            {
                auto& ClipArt = Options.m_Source.m_ClipArt[I];

                0
                || (Err = Stream.Field("Path",          ClipArt.m_SourcePath ))
                || (Err = Stream.Field("UnicodeChar",   ClipArt.m_UnicodeChar))
                || (Err = Stream.Field("Size",          ClipArt.m_Size ))
                ;
            }) ) 
        {
            // If we don't have this record is because there is no clip art (Which is ok)
            if( Error.getCode().getState<xcore::textfile::error_state>() == xcore::textfile::error_state::UNEXPECTED_RECORD ) Error.clear();
            else return Error;
        }


        if (Stream.Record(Error, "Generation"
            , [&](std::size_t, xcore::err& Err)
            {   
                int  nClips = static_cast<int>(Options.m_Source.m_ClipArt.size());

                xcore::cstring Type = descriptor::type_v[(int) Options.m_Generation.m_Type ];

                0
                || (Err = Stream.Field("BorderSize",            Options.m_Generation.m_BorderSize))
                || (Err = Stream.Field("BlurSize",              Options.m_Generation.m_BLurSize))
                || (Err = Stream.Field("Type",                  Type  ))
                ;

                if( isRead )
                {
                    for( int i=0; auto& E : descriptor::type_v ) 
                        if( E == Type )
                        { 
                            Options.m_Generation.m_Type = static_cast<descriptor::type>(i); 
                            break; 
                        } 
                        else ++i;

                    Options.m_Source.m_ClipArt.appendList(nClips);
                }

            })) return Error;

        return {};
    }

}
