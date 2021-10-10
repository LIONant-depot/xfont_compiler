
#include "../../dependencies/msdfgen/msdfgen.h"
#include "../../dependencies/msdfgen/msdfgen-ext.h"
#include "../../dependencies/xbmp_tools/src/xbmp_tools.h"
#include "../../src_runtime/xfont.h"

#include "xfont_PerfectHashing.cpp"

#pragma comment( lib, "../../dependencies/msdfgen/freetype/win64/freetype.lib")

namespace xfont_compiler {

using namespace msdfgen;

struct implementation : instance
{
    struct sprite
    {
        xcore::bitmap                   m_Bitmap;
        xcore::vector2                  m_Hotpoint;     // 0,0 relative
        xcore::guid::unit<32>           m_Guid;
        std::array<xfont::vertex, 4 >   m_Vertices;
        std::array<std::uint32_t,6>     m_Indices { 0, 1, 3, 0, 2, 1 };
    };

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

    void FontToSprites(const descriptor& Descriptor, xresource_pipeline::compiler::base::optimization_type Optimization)
    {
        if(Descriptor.m_Generation.m_Type == descriptor::type::MSDF)
        {
            //m_MSDFConfig.GeneratorConfig(true);
        }

        const_cast<int&>(Descriptor.m_Generation.m_BorderSize) = 2;

        if( m_pFont )
        {
            auto end_i = xcore::string::Length(m_Text).m_Value;
            m_SpriteList.reserve(end_i);

            int i=0;
            for( auto i=0u; i<end_i; ++i )
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

                        auto& Sprite = m_SpriteList.append();
                        Sprite.m_Guid.m_Value = std::as_const(m_Text)[i];

                        BitmapData[0].m_Value = 0;
                        Sprite.m_Bitmap.setupFromColor( w, H, { BitmapData.release(), w*H+1u }, true );

                        //XBitmap.SaveTGA( xcore::string::To<wchar_t>(xcore::string::Fmt("x64/FontChar%04d.tga", i)) );
                    }
                    else if( Descriptor.m_Generation.m_Type == descriptor::type::MTSDF )
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

                        auto& Sprite = m_SpriteList.append();
                        Sprite.m_Guid.m_Value = m_Text[i];

                        BitmapData[0].m_Value = 0;
                        Sprite.m_Bitmap.setupFromColor( w, H, { BitmapData.release(), w*H+1u }, true );

                      //  XBitmap.SaveTGA( xcore::string::To<wchar_t>(xcore::string::Fmt("x64/FontChar%04d.tga", i)) );
                    }
                    else if( Descriptor.m_Generation.m_Type == descriptor::type::SDF )
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

                        auto& Sprite = m_SpriteList.append();
                        Sprite.m_Guid.m_Value = m_Text[i];

                        BitmapData[0].m_Value = 0;
                        Sprite.m_Bitmap.setupFromColor( w, H, { BitmapData.release(), w*H+1u }, true );

                      //  XBitmap.SaveTGA( xcore::string::To<wchar_t>(xcore::string::Fmt("x64/FontChar%04d.tga", i)) );
                    }
                    else
                    {
                        // Has no type...
                        xassert(false);
                    }

                }
            }
            
        }
    }

    //---------------------------------------------------------------------------------------

    void CompactifySprite( sprite& SpriteCore ) const
    {
        const auto BW   = SpriteCore.m_Bitmap.getWidth();
        const auto BH   = SpriteCore.m_Bitmap.getHeight();
        const auto Data = SpriteCore.m_Bitmap.getMip<xcore::icolor>(0);
        std::uint32_t  t,b,l,r;
        
        //
        // From Top
        //
        for( t=0; t<BH; t++ )
        {
            bool bFoundSomething=false;
            for( std::uint32_t x=0; x<BW; x++ )
            {
                // Any value in the alpha will trigger the stop condition
                if( Data[ t*BW + x ].m_A )
                {
                    bFoundSomething = true;
                    break;
                }
            }
            if( bFoundSomething ) break;
        }
        
        //
        // From Bottom
        //
        for( b=BH-1; b>= 0; b-- )
        {
            bool bFoundSomething= false;
            for(std::uint32_t x=0; x<BW; x++ )
            {
                // Any value in the alpha will trigger the stop condition
                if( Data[ b*BW + x ].m_A )
                {
                    bFoundSomething = true;
                    break;
                }
            }
            if( bFoundSomething ) break;
        }
        
        //
        // From Left
        //
        for( l=0; l<BW; l++ )
        {
            bool bFoundSomething=false;
            for(std::uint32_t y=t; y<=b; y++ )
            {
                // Any value in the alpha will trigger the stop condition
                if( Data[ y*BW + l ].m_A )
                {
                    bFoundSomething = true;
                    break;
                }
            }
            if( bFoundSomething ) break;
        }
        
        //
        // From Right
        //
        for( r=BW-1; r>=0; r-- )
        {
            bool bFoundSomething=false;
            for(std::uint32_t y=t; y<=b; y++ )
            {
                // Any value in the alpha will trigger the stop condition
                if( Data[ y*BW + r ].m_A )
                {
                    bFoundSomething = true;
                    break;
                }
            }
            if( bFoundSomething ) break;
        }
        
        //
        // Clip Bitmap Base on our findings
        //
        
        // If we dont have anything to clip bail out
        if( l==0 && t==0 && r==BW-1 && b==BH-1 )
            return;
        
        // Edge case where it found actually no pixels
        if( r < 0 )
        {
            SpriteCore.m_Bitmap.CreateBitmap(1, 1);
            SpriteCore.m_Bitmap.getMip<std::uint32_t>(0)[0] = 0;
            
            SpriteCore.m_Vertices[0].m_X = 0;
            SpriteCore.m_Vertices[0].m_Y = 0;
            SpriteCore.m_Vertices[1].m_X = 0;
            SpriteCore.m_Vertices[1].m_Y = 0;
            SpriteCore.m_Vertices[2].m_X = 0;
            SpriteCore.m_Vertices[2].m_Y = 0;
            SpriteCore.m_Vertices[3].m_X = 0;
            SpriteCore.m_Vertices[3].m_Y = 0;

            return;
        }
            
        //
        // make into a new bitmap
        //
        
        xcore::bitmap NewBitmap;
        const auto     NW = r-l+1;
        const auto     NH = b-t+1;
        
        NewBitmap.CreateBitmap( NW, NH );
        auto NewData = NewBitmap.getMip<xcore::icolor>(0);
        
        for(std::uint32_t y=t, y1=0; y1<NH; y++, y1++ )
        for(std::uint32_t x=l, x1=0; x1<NW; x++, x1++ )
        {
            NewData[ x1 + y1*NW ] = Data[ x + y*BW ];
        }
        
        //
        // Replace the old with the new
        //
        SpriteCore.m_Bitmap.Copy( NewBitmap );
        
        
        //
        // Handle the vertices
        //

        // This crazy formula is to compensate for the OPENGL YAxis Flip
        {
      //      auto d = t;
      //      t = BH - b;
      //      b = BH - d;
        }
        
        // set the final coordinates
        SpriteCore.m_Vertices[ 0 ].m_X = (float)l;
        SpriteCore.m_Vertices[ 0 ].m_Y = (float)t;
        SpriteCore.m_Vertices[ 1 ].m_X = (float)r;
        SpriteCore.m_Vertices[ 1 ].m_Y = (float)t;
        SpriteCore.m_Vertices[ 2 ].m_X = (float)r;
        SpriteCore.m_Vertices[ 2 ].m_Y = (float)b;
        SpriteCore.m_Vertices[ 3 ].m_X = (float)l;
        SpriteCore.m_Vertices[ 3 ].m_Y = (float)b;
    }

    //---------------------------------------------------------------------------------------------

    void SpritesToAtlas( void )
    {
        //
        // Create all bins & Set the vertex positions
        //
        auto Rectangles     = std::make_unique<xbmp::tools::atlas::rect_xywhf[]>(m_SpriteList.size()) ;
        auto RectanglesPtrs = std::make_unique<xbmp::tools::atlas::rect_xywhf*[]>(m_SpriteList.size());
        for( auto i = 0u; i< m_SpriteList.size(); ++i )
        {
            auto& Sprite = m_SpriteList[i];
            auto& Rec    = Rectangles[i];

            Rec = xbmp::tools::atlas::rect_xywhf( 0, 0, Sprite.m_Bitmap.getWidth(), Sprite.m_Bitmap.getHeight() );
            RectanglesPtrs[i] = &Rec;

            Sprite.m_Vertices[0].m_X = 0;
            Sprite.m_Vertices[0].m_Y = 0;

            Sprite.m_Vertices[1].m_X = 0;
            Sprite.m_Vertices[1].m_Y = 0;

            Sprite.m_Vertices[2].m_X = 0;
            Sprite.m_Vertices[2].m_Y = 0;

            Sprite.m_Vertices[3].m_X = 0;
            Sprite.m_Vertices[3].m_Y = 0;
        }

        //
        // Atlas
        //
        xcore::vector<xbmp::tools::atlas::bin> BinList;
        xbmp::tools::atlas                     Atlas;

        bool bSuccess = Atlas.Pack(RectanglesPtrs.get(), (int)m_SpriteList.size(), 1024, BinList );

        //
        // Generate the atlas
        //
        auto&           TopBin          = BinList[0];
        
        m_AtlasTexture.CreateBitmap(TopBin.m_Size.m_W, TopBin.m_Size.m_H );

              auto AtlasData = m_AtlasTexture.getMip<xcore::icolor>(0);
        const auto AW        = m_AtlasTexture.getWidth();
        const auto AH        = m_AtlasTexture.getHeight();

        //
        // Copy bitmaps to atlas
        //
        for( auto i = 0ull, end_i = TopBin.m_Rects.size(); i < end_i; i++)
        {
            const auto& Rect        = *TopBin.m_Rects[i];
            const auto  Index       = xcore::containers::getIndex<int>( Rect, Rectangles.get() );
                  auto& Vertex      = m_SpriteList[Index].m_Vertices;
            const auto& Bitmap      = m_SpriteList[Index].m_Bitmap;
            const auto  BitmapData  = Bitmap.getMip<xcore::icolor>(0);
            const auto  BW          = Bitmap.getWidth();
            const auto  BH          = Bitmap.getHeight();
            
            if(Rect.m_bFlipped)
            {
                xassert(BH == Rect.getWidth());
                xassert(BW == Rect.getHeight());

                // Need to write the code for the flip case....
                for( auto y=0u; y<BW; y++ )
                for( auto x=0u; x<BH; x++ )
                {
                    AtlasData[ (Rect.m_X + x) + (Rect.m_Y + y)*AW ] = BitmapData[ (BH-x-1) * BW + y ];
                }

                Vertex[0].m_U  = (Rect.m_X +  0) / (float)AW;
                Vertex[0].m_V  = (Rect.m_Y +  0) / (float)AH;
                
                Vertex[1].m_U  = (Rect.m_X +  0) / (float)AW;
                Vertex[1].m_V  = (Rect.m_Y + BW) / (float)AH;
                
                Vertex[2].m_U  = (Rect.m_X + BH) / (float)AW;
                Vertex[2].m_V  = (Rect.m_Y + BW) / (float)AH;
                
                Vertex[3].m_U  = (Rect.m_X + BH) / (float)AW;
                Vertex[3].m_V  = (Rect.m_Y +  0) / (float)AH;
            }
            else
            {
                xassert(BW == Rect.getWidth());
                xassert(BH == Rect.getHeight());

                for( auto y=0u; y<BH; y++ )
                for( auto x=0u; x<BW; x++ )
                {
                    AtlasData[ (Rect.m_X + x) + (Rect.m_Y + y)*AW ] = BitmapData[ x + y * BW ];
                }
            
                Vertex[0].m_U = (Rect.m_X +  0) / (float)AW;
                Vertex[0].m_V = (Rect.m_Y + BH) / (float)AH;

                Vertex[1].m_U = (Rect.m_X + BW) / (float)AW;
                Vertex[1].m_V = (Rect.m_Y + BH) / (float)AH;

                Vertex[2].m_U = (Rect.m_X + BW) / (float)AW;
                Vertex[2].m_V = (Rect.m_Y +  0) / (float)AH;

                Vertex[3].m_U = (Rect.m_X +  0) / (float)AW;
                Vertex[3].m_V = (Rect.m_Y +  0) / (float)AH;
            }
        }
    }

    //---------------------------------------------------------------------------------------------

    void ComputeHashTable( void )
    {
        //
        // Copy all the guids into its own memory
        //
        xcore::unique_span<std::uint32_t> Keys;
        Keys.New(m_SpriteList.size()).CheckError();
        for( const auto& E : m_SpriteList )
        {
            const auto Index = xcore::containers::getIndex<int>(E, m_SpriteList);
            Keys[Index] = E.m_Guid.m_Value;
        }

        //
        // Compute a perfect hash function for it
        //
        std::uint32_t                     HashTableSize;
        {
            buid_perfect_hashing Perfect;
            Perfect.Compile(HashTableSize, m_PHTable, Keys);
        }

        //
        // Create a hash function and insert all the sprites
        //
        m_HashTable.Alloc(HashTableSize).CheckError();
        std::memset( m_HashTable.data(), 0xff, m_HashTable.getByteSize() );
        
        for ( const auto& E : m_SpriteList)
        {
            const auto Key    = E.m_Guid.m_Value;
            const auto Row    = Key / m_PHTable.size();
            const auto Column = Key % m_PHTable.size();
            const auto iHash  = Column + m_PHTable[Row];
            
            m_HashTable[iHash] = xcore::containers::getIndex<std::uint32_t>(E, m_SpriteList);
        }

        //
        // Test how given a key we can get back the sprite index
        //
        if constexpr (false)
        {
            xcore::random::small_generator Small;
            for( int i=0; i<100; ++i )
            {
                const std::uint32_t FindIndex       = Small.RandU32()%m_SpriteList.size();
                const std::uint32_t Key             = m_SpriteList[FindIndex].m_Guid.m_Value;
                const auto          Row             = Key / m_PHTable.size();
                const auto          Column          = Key % m_PHTable.size();
                const auto          iHash           = Column + m_PHTable[Row];
                const auto          IndexFromKey    = m_HashTable[iHash];

                xassert( FindIndex == IndexFromKey );
            }
        }
    }

    //---------------------------------------------------------------------------------------------

    void BuildRuntimeStructure( void )
    {
        //
        // Count verts and Indices
        //
        std::uint32_t nVertices = 0;
        std::uint32_t nIndices  = 0;
        std::uint32_t nMeshes   = 0;
        for( const auto& E : m_SpriteList )
        {
            nVertices += static_cast<std::uint32_t>(E.m_Vertices.size());
            nIndices  += static_cast<std::uint32_t>(E.m_Indices.size());
            nMeshes++;
        }

        //
        // Allocate all structures
        //
        m_Indices.Alloc(nIndices).CheckError();
        m_Vertices.Alloc(nVertices).CheckError();
        m_Meshes.Alloc(nMeshes).CheckError();

        //
        // Copy all the data
        //
        nVertices = 0;
        nIndices  = 0;
        nMeshes   = 0;
        for (const auto& E : m_SpriteList)
        {
            for( const auto& V : E.m_Vertices )
            {
                m_Vertices[nVertices++] = V;
            }

            m_Meshes[nMeshes++] = xfont::mesh
            { .m_iIndex = nIndices
            , .m_Count  = static_cast<std::uint16_t>(E.m_Indices.size())
            };

            for (const auto& I : E.m_Indices )
            {
                m_Indices[nIndices++] = I;
            }
        }

        //
        // Set the final structure
        //
        m_Font.m_pHash      = m_HashTable.data();
        m_Font.m_pPHHash    = m_PHTable.data();
        m_Font.m_pVertices  = m_Vertices.data();
        m_Font.m_pIndices   = m_Indices.data();
        m_Font.m_pMesh      = m_Meshes.data();

        m_Font.m_nIndices   = static_cast<std::uint32_t>(m_Indices.size());
        m_Font.m_nVertices  = static_cast<std::uint32_t>(m_Vertices.size());
        m_Font.m_nMeshes    = static_cast<std::uint32_t>(m_Meshes.size());
        m_Font.m_nHash      = static_cast<std::uint32_t>(m_HashTable.size());
        m_Font.m_nPHHash    = static_cast<std::uint32_t>(m_PHTable.size());
    }

    //---------------------------------------------------------------------------------------------

    virtual void    Compile(const descriptor& Descriptor, xresource_pipeline::compiler::base::optimization_type Optimization) override
    {
        FontToSprites(Descriptor, Optimization);
        for(auto& E : m_SpriteList ) CompactifySprite(E);
        SpritesToAtlas();
        ComputeHashTable();
        BuildRuntimeStructure();
    }

    //---------------------------------------------------------------------------------------------

    virtual void    Serialize(const std::string_view FilePath) override
    {
        xcore::serializer::stream Stream;

        if( auto Err = Stream.Save( xcore::string::To<wchar_t>(FilePath), m_Font ); Err )
            throw( std::runtime_error("Failed to serialize the font") );
    }

    //---------------------------------------------------------------------------------------------

    ~implementation()
    {
        if(m_pFont) destroyFont(m_pFont);
        if(m_pFreeType) deinitializeFreetype(m_pFreeType);
    }

    //---------------------------------------------------------------------------------------------

    FreetypeHandle*                     m_pFreeType     = initializeFreetype();
    FontHandle*                         m_pFont         = nullptr;
    xcore::wstring                      m_Text          {};
    MSDFGeneratorConfig                 m_MSDFConfig    {};
    xcore::vector<sprite>               m_SpriteList    {};
    xcore::bitmap                       m_AtlasTexture  {};
    xcore::unique_span<std::uint32_t>   m_HashTable     {};
    xcore::unique_span<std::uint32_t>   m_PHTable       {};
    xfont                               m_Font          {};
    xcore::unique_span<std::uint32_t>   m_Indices       {};
    xcore::unique_span<xfont::vertex>   m_Vertices      {};
    xcore::unique_span<xfont::mesh>     m_Meshes        {};
};

//---------------------------------------------------------------------------------------------
// MAKE INSTANCE
//---------------------------------------------------------------------------------------------
std::unique_ptr<instance> MakeInstance()
{
    return std::make_unique<implementation>();
}

}// end of namespace xfont_compiler
