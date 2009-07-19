
void WriteCfgString   (char *Section,char *Key,char *Value) ;
void WriteCfgInt      (char *Section,char *Key,int Value) ;
void ReadCfgString    (char *Section,char *Key,char *DefaultValue,char *retValue) ;
int ReadCfgInt        (char *Section,char *Key,int DefaultValue) ;

 
void LoadConfig()  ;
void SaveConfig()  ; 
    
