
#include "../../msdfgen/msdfgen.h"
#include "../../msdfgen/msdfgen-ext.h"

#pragma comment( lib, "../../dependencies/msdfgen/freetype/win64/freetype.lib")

namespace xfont_compiler {

using namespace msdfgen;

struct implementation : instance
{
    //---------------------------------------------------------------------------------------------

    virtual void LoadFont(const xcore::cstring& AssetPath, const descriptor& Descriptor) override
    {
        //
        // Read the font with the file
        //
        if( false == Descriptor.m_Source.m_Font.m_SourcePath.empty() )
        {
            m_pFont = loadFont(m_pFreeType, xcore::string::Fmt( "%s/%s", AssetPath.data(), Descriptor.m_Source.m_Font.m_SourcePath.data() ) );
            if(m_pFont == nullptr )
                throw( std::runtime_error( "Failed to load the font") );

            if( false == Descriptor.m_Source.m_Font.m_UnicodeTxtFilePath.empty() )
            {
                xcore::file::stream File;

                if( auto Err = File.open( xcore::string::To<wchar_t>(xcore::string::Fmt("%s/%s", AssetPath.data(), Descriptor.m_Source.m_Font.m_SourcePath.data())), "rb" ); Err )
                    throw(std::runtime_error("Failed to open the unicode textfile for the given font"));


                xcore::units::bytes Bytes;
                if( auto Err = File.getFileLength(Bytes); Err ) 
                    throw(std::runtime_error( Err.getCode().m_pString ) );
                xcore::wstring TextFile( xcore::wstring::units{ std::uint32_t(Bytes.m_Value/2 + 1) } );

                if( auto Err = File.ReadView( TextFile.getView() ); Err )
                    throw(std::runtime_error("Failed to read the unicode textfile for the given font"));

                m_Text = std::move(TextFile);
            }
            else
            {
                m_Text = xcore::wstring( xcore::string::constant{ L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" } );
            }
        }

        //
        // Read Clip Art
        //
        //....
    }

    //---------------------------------------------------------------------------------------------

    virtual void    Compile(const descriptor& Descriptor, xresource_pipeline::compiler::base::optimization_type Optimization) override
    {
        if(Descriptor.m_Generation.m_Type == descriptor::type::MSDF)
        {
            //m_MSDFConfig.GeneratorConfig(true);
        }

        const_cast<int&>(Descriptor.m_Generation.m_BorderSize) = 2;

        if( m_pFont )
        {
            int i=0;
            for( int i=0, end_i = xcore::string::Length(m_Text).m_Value; i<end_i; ++i )
            {
                Shape LetterShape;
                if( loadGlyph( LetterShape, m_pFont, std::as_const(m_Text)[xcore::wstring::units{(std::uint32_t)i}]))
                {
                    LetterShape.normalize();

                    Shape::Bounds bounds = LetterShape.getBounds(Descriptor.m_Generation.m_BorderSize);

                    const int h = int(bounds.t - bounds.b);
                    const int w = int(bounds.r - bounds.l);

                    Projection          FinalProjection(Vector2(1, 1), msdfgen::Vector2(-bounds.l, -bounds.b));

                    edgeColoringSimple(LetterShape, 3.0);

                    if( Descriptor.m_Generation.m_Type == descriptor::type::MSDF )
                    {
                        Bitmap<float, 3> BitmapMSDF(Descriptor.m_Source.m_Font.m_Size, Descriptor.m_Source.m_Font.m_Size);
                        generateMSDF( BitmapMSDF, LetterShape, FinalProjection, Descriptor.m_Generation.m_BLurSize, m_MSDFConfig );
                        // savePng(BitmapMSDF, xcore::string::Fmt("x64/FontChar%04d.png", i).data());

                        const int H = h > Descriptor.m_Source.m_Font.m_Size ? Descriptor.m_Source.m_Font.m_Size : h;
                        auto BitmapData = std::make_unique<xcore::icolor[]>( w * H + 1 );

                        for( int y=0; y<H; ++y )
                        for( int x=0; x<w; ++x )
                        {
                            const int Y1 = Descriptor.m_Source.m_Font.m_Size - y - 1 - (Descriptor.m_Source.m_Font.m_Size- H);
                            const int Y2 = y;
                            const int X2 = 1 + x;
                            const int X1 = x;
                            xassert( Y1 >= 0 );
                            BitmapData[ X2 + Y2 * w ].setupFromRGB( xcore::vector3d( BitmapMSDF(X1,Y1)[0], BitmapMSDF(X1, Y1)[1], BitmapMSDF(X1, Y1)[2]) );
                        }
                        xcore::bitmap XBitmap;
                        BitmapData[0].m_Value = 0;
                        XBitmap.setupFromColor( w, H, { BitmapData.release(), w*H+1u }, true );

                        //XBitmap.SaveTGA( xcore::string::To<wchar_t>(xcore::string::Fmt("x64/FontChar%04d.tga", i)) );
                    }

                    if( Descriptor.m_Generation.m_Type == descriptor::type::MTSDF )
                    {
                        Bitmap<float, 4> BitmapMTSDF(Descriptor.m_Source.m_Font.m_Size, Descriptor.m_Source.m_Font.m_Size);
                        generateMTSDF( BitmapMTSDF, LetterShape, FinalProjection, Descriptor.m_Generation.m_BLurSize, m_MSDFConfig );
                       // savePng(BitmapMTSDF, xcore::string::Fmt("x64/FontChar%04d.png", i).data());

                        const int H = h > Descriptor.m_Source.m_Font.m_Size ? Descriptor.m_Source.m_Font.m_Size : h;
                        auto BitmapData = std::make_unique<xcore::icolor[]>( w * H + 1 );

                        for( int y=0; y<H; ++y )
                        for( int x=0; x<w; ++x )
                        {
                            const int Y1 = Descriptor.m_Source.m_Font.m_Size - y - 1 - (Descriptor.m_Source.m_Font.m_Size- H);
                            const int Y2 = y;
                            const int X2 = 1 + x;
                            const int X1 = x;
                            xassert( Y1 >= 0 );
                            BitmapData[ X2 + Y2 * w ].setupFromRGBA( xcore::vector4(BitmapMTSDF(X1,Y1)[0], BitmapMTSDF(X1, Y1)[1], BitmapMTSDF(X1, Y1)[2], BitmapMTSDF(X1, Y1)[3]) );
                        }
                        xcore::bitmap XBitmap;
                        BitmapData[0].m_Value = 0;
                        XBitmap.setupFromColor( w, H, { BitmapData.release(), w*H+1u }, true );

                      //  XBitmap.SaveTGA( xcore::string::To<wchar_t>(xcore::string::Fmt("x64/FontChar%04d.tga", i)) );
                    }

                    if( Descriptor.m_Generation.m_Type == descriptor::type::SDF )
                    {
                        Bitmap<float, 1> BitmapSDF(Descriptor.m_Source.m_Font.m_Size, Descriptor.m_Source.m_Font.m_Size);
                        generateSDF( BitmapSDF, LetterShape, FinalProjection, Descriptor.m_Generation.m_BLurSize, m_MSDFConfig );
                      //  savePng(BitmapSDF, xcore::string::Fmt("x64/FontChar%04d.png", i).data());

                        const int H = h > Descriptor.m_Source.m_Font.m_Size ? Descriptor.m_Source.m_Font.m_Size : h;
                        auto BitmapData = std::make_unique<xcore::icolor[]>( w * H + 1 );

                        for( int y=0; y<H; ++y )
                        for( int x=0; x<w; ++x )
                        {
                            const int Y1 = Descriptor.m_Source.m_Font.m_Size - y - 1 - (Descriptor.m_Source.m_Font.m_Size- H);
                            const int Y2 = y;
                            const int X2 = 1 + x;
                            const int X1 = x;
                            xassert( Y1 >= 0 );
                            BitmapData[ X2 + Y2 * w ].setupFromRGB( xcore::vector3d(BitmapSDF(X1,Y1)[0], BitmapSDF(X1, Y1)[0], BitmapSDF(X1, Y1)[0]) );
                        }
                        xcore::bitmap XBitmap;
                        BitmapData[0].m_Value = 0;
                        XBitmap.setupFromColor( w, H, { BitmapData.release(), w*H+1u }, true );

                      //  XBitmap.SaveTGA( xcore::string::To<wchar_t>(xcore::string::Fmt("x64/FontChar%04d.tga", i)) );
                    }

                }
            }
            
        }
    }

    //---------------------------------------------------------------------------------------------

    virtual void    Serialize(const std::string_view FilePath) override
    {
    
    }

    ~implementation()
    {
        if(m_pFont) destroyFont(m_pFont);
        if(m_pFreeType) deinitializeFreetype(m_pFreeType);
    }

    FreetypeHandle*         m_pFreeType  = initializeFreetype();
    FontHandle*             m_pFont      = nullptr;
    xcore::wstring          m_Text       {};
    MSDFGeneratorConfig     m_MSDFConfig {};
};

//---------------------------------------------------------------------------------------------
// MAKE INSTANCE
//---------------------------------------------------------------------------------------------
std::unique_ptr<instance> MakeInstance()
{
    return std::make_unique<implementation>();
}

}// end of namespace xfont_compiler
