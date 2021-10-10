//
// To hash into the right place do the following:
// const int Row    = Key / PHTable.getCount();
// const int Column = Key % PHTable.getCount();
// const int iHash  = Column + PHTable[Row];
class buid_perfect_hashing
{
public:

    void                Compile             ( std::uint32_t& HashTableSize, xcore::unique_span<std::uint32_t>& PHTable, std::span<const std::uint32_t> Keys );
    
protected:
    
    struct row_struct
    {
        int m_RowNumber;                      // the row number in array A[][]
        int m_RowItemCnt;                     // the # of items in this row of A[][]
    };
    
    enum constants : std::uint32_t
    {
        INVALID_KEY = 0xffffffff
    };
    
protected:
    
    void                InitArrays          ( void );
 //   void                SortRows            ( int t );
    
protected:
    
    std::uint32_t                 m_tMax;
    std::uint64_t                 m_HashTableMax;
    
    // the arrays A[][], r[] and C[] are those in the article "Perfect Hashing"
    xcore::unique_span<std::uint32_t> m_A;           // A[i][j]=K (i=K/t, j=K mod t) for each key K
    xcore::unique_span<int>           m_R;           // r[R]=amount row A[R][] was shifted
    xcore::unique_span<std::uint32_t> m_C;           // the shifted rows of A[][] collapse into C[]
    
    // Row[] exists to facilitate sorting the rows of A[][] by their "fullness"
    xcore::unique_span<row_struct>    m_Row;         // entry counts for the rows in A[][]
};