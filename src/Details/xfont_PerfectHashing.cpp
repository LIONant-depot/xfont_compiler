//
// Base on: http://www.drdobbs.com/architecture-and-design/generating-perfect-hash-functions/184404506?pgno=1
//

#include "xfont_PerfectHashing.h"

//--------------------------------------------------------------------------
// Overview: Prepares the algorithm data structures for use.
// Notes & Caveats:
//      -A row offset may be 0, so the items in r[] are set to a negative value to
//       indicate that the offset for each row is not known yet.
//      -Every item in A[][] and C[] is set to a value that is known to be an invalid
//       key for the specific application.  -1 is often a good choice.
//--------------------------------------------------------------------------
void buid_perfect_hashing::InitArrays( void )
{
    m_A.Alloc( m_tMax*m_tMax ).CheckError();
    m_R.Alloc( m_tMax ).CheckError();
    m_C.Alloc( m_HashTableMax ).CheckError();
    m_Row.Alloc( m_tMax ).CheckError();
    
    for( std::uint32_t row = 0; row < m_tMax; row++)
    {
        m_R[row]                = -1;        // valid offsets are non-negative
        m_Row[row].m_RowNumber  = row;       // insert the row numbers and
        m_Row[row].m_RowItemCnt = 0;         //  indicate that each row is empty
        
        for(std::uint32_t column = 0; column < m_tMax; column++ )
        {
            m_A[row * m_tMax + column] = INVALID_KEY;
        }
    }
    
    for(std::uint32_t row = 0; row < m_HashTableMax; row++ )
    {
        m_C[row] = INVALID_KEY;
    }
}

//--------------------------------------------------------------------------
// Parameters:
// int t            - the number of rows in A[][]; max(key) must be < t*t
// int *KeyCount    - pointer to location to place number of keys read
// Notes & Caveats:
//       -The number of items in each row is also computed and returned in
//         Row[row].RowItemCnt.
//       -The number of keys is returned to the caller via a pointer.  If an error
//         is detected the number of keys reflects how many keys were read before the
//         error condition was detected.
//--------------------------------------------------------------------------
void buid_perfect_hashing::Compile( std::uint32_t& HashTableSize, xcore::unique_span<std::uint32_t>& PHTable, std::span<const std::uint32_t> Keys)
{
    const std::uint32_t Count = static_cast<int>(Keys.size());

    //
    // Find the max value for our key
    //
    std::uint32_t MaxKey=0;
    for(std::uint32_t i=0; i<Count; i++)
    {
        if( Keys[i] > MaxKey ) MaxKey = Keys[i];
    }

    m_tMax          = 1 + static_cast<std::uint32_t>(std::sqrt( MaxKey ));
    m_HashTableMax  = MaxKey+m_tMax;
    
    //
    // Initialize all the arrays
    //
    InitArrays();
    
    //
    // Insert all the keys into the tables
    //
    for(std::uint32_t i=0; i<Count; i++)
    {
        const std::uint32_t Key    = Keys[i];
        const std::uint32_t Row    = Key / m_tMax;
        const std::uint32_t Column = Key % m_tMax;
        
        xassert( Row < m_tMax );
        
        m_A[Row * m_tMax + Column] = Key;
        
        m_Row[Row].m_RowItemCnt++;
    }
    
    //      -The algorithm needs to know which row of A[][] is most full, 2nd most full,
    //          etc.  This is most easily done by sorting an array of row-item-counts and
    //          remembering which row the item counts go with.  That is what the array
    //          "struct RowStruct Row[]" does for us.
    //      -I saw no point in trying to be clever here, so a simple bubble sort is used.
    for( std::uint32_t i = 0; i < m_tMax-1; i++ )
    {
        for( std::uint32_t j = i+1; j < m_tMax; j++ )
        {
            if( m_Row[i].m_RowItemCnt < m_Row[j].m_RowItemCnt )
            {
                std::swap( m_Row[i], m_Row[j] );
            }
        }
    }
    
    // do the First-Fit Descending Method algorithm
    // For each non-empty row:
    // 1. shift the row right until none of its items collide with any of
    //    the items in previous rows.
    // 2. Record the shift amount in array r[].
    // 3. Insert this row into the hash table C[].
    for( std::uint32_t ndx = 0; ndx<m_tMax && m_Row[ndx].m_RowItemCnt > 0; ndx++ )
    {
        const int   Row = m_Row[ndx].m_RowNumber;        // get the next non-empty row
        int         offset;
        
        for( offset = 0; offset < m_HashTableMax - m_tMax - 1; offset++)
        {
            std::uint32_t k;
            
            // Search thought the offsets to find no collisions
            // does this offset avoids collisions?
            for( k = 0; k < m_tMax; k++ )
            {
                if( ( m_C[ offset+k ]        != INVALID_KEY ) &&
                    ( m_A[ Row * m_tMax + k] != INVALID_KEY ) ) break;
            }
            
            if( k == m_tMax )
            {
                // record the shift amount for this row
                m_R[Row] = offset;
                
                // insert this row into the hash table
                for( k = 0; k < m_tMax; k++ ) 
                {
                    if( m_A[Row * m_tMax + k] != INVALID_KEY )
                    {
                        m_C[ offset+k ] = m_A[ Row * m_tMax + k ];
                    }
                }
                break;
            }
        }
        
        // failed to fit row into the hash table
        // try increasing the m_HashTableMax
        xassert( offset != (m_HashTableMax - m_tMax -1) );
    }
    
    //
    // all done!  locate the "right-most" hash table entry
    //
    HashTableSize=0;
    for(std::uint32_t k = 0; k < m_HashTableMax; k++ )
    {
        if ( m_C[k] != INVALID_KEY ) HashTableSize = k+1;
    }
    
    //
    // Copy all the hash table information
    //
    PHTable.Alloc(m_tMax).CheckError();
    
    for(std::uint32_t k=0; k<m_tMax; k++ )
    {
        if( m_R[k] < 0 ) PHTable[k] = -1;
        else
        {
            const int Value = m_R[k];
            PHTable[k] = Value;
            xassert(PHTable[k] == m_R[k]);
        }
    }
    
    //
    // Final Sanity Check
    //
    for( std::uint32_t i=0; i<Keys.size(); i++ )
    {
        const auto Key    = Keys[i];
        const auto Row    = static_cast<std::uint32_t>(Key / PHTable.size());
        const auto Column = static_cast<std::uint32_t>(Key % PHTable.size());
        const auto Index  = PHTable[Row];
        const auto iHash  = Column+Index;
        
        xassert(iHash < HashTableSize );
        const int Value  = m_C[ iHash ];

        xassert( Key == Value );
    }
    
    //
    // print the results
    //
    printf("\nINFO: TableUsage(%.1f%%) TableSize(%d) nKeys(%d) MaxKeyValue(%d)\n",
             100.0*Count/HashTableSize,
             HashTableSize,
             Count,
             m_tMax);
    
    if((0))
    {
        printf("\noffset table r[]\n");
        printf("row offset\n");
        for(std::uint32_t k = 0; k < m_tMax; k++)
        {
            printf("%2d  %3d\n", k, m_R[k]);
        }
        
        printf("\nhash table C[]\n");
        for(std::uint32_t k = 0; k < HashTableSize; k++)
        {
            printf("%d\n", m_C[k]);
        }
    }
    
}


