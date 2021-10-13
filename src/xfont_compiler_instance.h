namespace xfont_compiler
{
    struct instance 
    {
        virtual        ~instance        ( void                                  ) = default;
        virtual void    LoadFont        ( xcore::vector<xcore::cstring>& AssetDependencies, const xcore::cstring& AssetPath, const descriptor& Descriptor ) = 0;
        virtual void    Compile         ( const descriptor& Descriptor, xresource_pipeline::compiler::base::optimization_type Optimization, xcore::guid::rcfull<> AtlasResourceGuid ) = 0;
        virtual void    SerializeAtlas  ( const std::string_view PathToTheVirtualResourceAtlas, const std::string_view FileName ) = 0;
        virtual void    Serialize       ( const std::string_view FilePath ) = 0;
    };

    std::unique_ptr<instance> MakeInstance();
}
