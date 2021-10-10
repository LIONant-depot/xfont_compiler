#ifndef XFONT_COMPILER_RUNTIME_H
#define XFONT_COMPILER_RUNTIME_H
#pragma once

#include "xcore.h"

struct xfont
{
    enum
    {
        VERSION = 1
    };

    struct vertex
    {
        float m_X, m_Y;
        float m_U, m_V;
    };

    struct mesh
    {
        std::uint32_t m_iIndex;
        std::uint16_t m_Count;
    };

    inline
    const mesh&                     getMeshByGuid   ( xcore::guid::unit<32> Guid ) const noexcept;
    inline
    std::span<const std::uint32_t>  getIndices      ( void ) const noexcept;
    inline
    std::span<const vertex>         getVertices     ( void ) const noexcept;
    inline 
    std::size_t                     getDataSize     ( void ) const noexcept;


    std::uint32_t*          m_pIndices;
    vertex*                 m_pVertices;
    mesh*                   m_pMesh;
    std::uint32_t*          m_pPHHash;
    std::uint32_t*          m_pHash;

    xcore::guid::rcfull<>   m_AtlasRcfull;
    std::uint32_t           m_nIndices;
    std::uint32_t           m_nVertices;
    std::uint32_t           m_nMeshes;
    std::uint32_t           m_nPHHash;
    std::uint32_t           m_nHash;
};

//----------------------------------------------------------------------------

const xfont::mesh& xfont::getMeshByGuid(xcore::guid::unit<32> Guid) const noexcept
{
    const auto Row      = Guid.m_Value / m_nPHHash;
    const auto Column   = Guid.m_Value % m_nPHHash;
    const auto iHash    = Column + m_pPHHash[Row];
    const auto iMesh    = m_pHash[iHash];
    return m_pMesh[ iMesh ];
}

//----------------------------------------------------------------------------

std::span<const std::uint32_t> xfont::getIndices( void ) const noexcept
{
    return { m_pIndices, m_nIndices };
}

//----------------------------------------------------------------------------

std::span<const xfont::vertex> xfont::getVertices( void ) const noexcept
{
    return { m_pVertices, m_nVertices };
}

//----------------------------------------------------------------------------

std::size_t xfont::getDataSize( void ) const noexcept
{
    return m_nIndices * sizeof(std::uint32_t) + m_nVertices * sizeof(vertex);
}

//----------------------------------------------------------------------------

namespace xcore::serializer::io_functions
{
    //-----------------------------------------------------------------------

    template<> inline
    xcore::err SerializeIO<xfont::vertex>(xcore::serializer::stream& Stream, const xfont::vertex& Vertex ) noexcept
    {
        xcore::err Err;
        false
        || ( Err = Stream.Serialize(Vertex.m_X))
        || ( Err = Stream.Serialize(Vertex.m_Y))
        || ( Err = Stream.Serialize(Vertex.m_U))
        || ( Err = Stream.Serialize(Vertex.m_V))
        ;
        return Err;
    }

    //-----------------------------------------------------------------------

    template<> inline
    xcore::err SerializeIO<xfont::mesh>(xcore::serializer::stream& Stream, const xfont::mesh& Mesh) noexcept
    {
        xcore::err Err;
        false
        || ( Err = Stream.Serialize(Mesh.m_iIndex) )
        || ( Err = Stream.Serialize(Mesh.m_Count)  )
        ;
        return Err;
    }

    //-----------------------------------------------------------------------

    template<> inline
    xcore::err SerializeIO<xfont>(xcore::serializer::stream& Stream, const xfont& Font ) noexcept
    {
        xcore::err Err;

        Stream.setResourceVersion( xfont::VERSION );

        false
        || ( Err = Stream.Serialize( Font.m_pVertices, Font.m_nVertices ))
        || ( Err = Stream.Serialize( Font.m_pIndices,  Font.m_nIndices  ))
        || ( Err = Stream.Serialize( Font.m_pMesh,     Font.m_nMeshes   ))
        || ( Err = Stream.Serialize( Font.m_pPHHash,   Font.m_nPHHash   ))
        || ( Err = Stream.Serialize( Font.m_pHash,     Font.m_nHash     ))

        || ( Err = Stream.Serialize( Font.m_AtlasRcfull.m_Instance.m_Value ))
        || ( Err = Stream.Serialize( Font.m_AtlasRcfull.m_Type.m_Value     ))

        || ( Err = Stream.Serialize( Font.m_nIndices  ))
        || ( Err = Stream.Serialize( Font.m_nVertices ))
        || ( Err = Stream.Serialize( Font.m_nMeshes   ))
        || ( Err = Stream.Serialize( Font.m_nPHHash   ))
        || ( Err = Stream.Serialize( Font.m_nHash     ))
        ;
        
        return Err;
    }
}

#endif