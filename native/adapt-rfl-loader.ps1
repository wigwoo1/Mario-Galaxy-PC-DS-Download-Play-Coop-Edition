param([Parameter(Mandatory=$true)][string]$Source)
$ErrorActionPreference='Stop'
function Replace-Function([string]$text,[string]$signature,[string]$replacement){
 $begin=$text.IndexOf($signature);if($begin-lt 0-or $text.IndexOf($signature,$begin+1)-ge 0){throw "RFL loader method anchor: $signature"}
 $open=$text.IndexOf('{',$begin);$end=$open+1;$depth=1
 while($depth-and $end-lt $text.Length){if($text[$end]-eq '{'){$depth++};if($text[$end]-eq '}'){$depth--};++$end}
 if($depth){throw 'Unbalanced RFL loader method'}
 return $text.Substring(0,$begin)+$replacement+$text.Substring($end)
}
$Source=Replace-Function $Source 'static void parseOnmemoryRes_(void) NO_INLINE {' @'
static BOOL parseOnmemoryRes_(void) NO_INLINE {
 RFLiLoader* loader=RFLiGetLoader();NativeRflHeader header;
 if(!loader||!nativeRflParse(loader->cache,loader->cacheSize,&header))return FALSE;
 loader->version=header.version;
 for(unsigned i=0;i<RFLiArcID_Max;++i){
  loader->archives[i].numFiles=header.sections[i].count;
  loader->archives[i].biggestSize=header.sections[i].largest;
  loader->archives[i].offset=header.sections[i].table;
 }
 return TRUE;
}
'@
$Source=Replace-Function $Source 'static u32 getCachedLength_(' @'
static u32 getCachedLength_(RFLiLoader* loader,u32 arcIdx,u16 fileIdx){
 uint32_t offset,length;
 return loader&&nativeRflFileSpan(loader->cache,loader->cacheSize,arcIdx,fileIdx,&offset,&length)?length:0;
}
'@
$Source=Replace-Function $Source 'static void* getCachedFile_(' @'
static void* getCachedFile_(void* dst,RFLiLoader* loader,u32 arcIdx,u16 fileIdx){
 uint32_t offset,length;
 if(!dst||!loader||!nativeRflFileSpan(loader->cache,loader->cacheSize,arcIdx,fileIdx,&offset,&length))return NULL;
 memcpy(dst,(const u8*)loader->cache+offset,length);return dst;
}
'@
$call='        parseOnmemoryRes_();'
if(-not $Source.Contains($call)){throw 'RFL cached resource parse call anchor'}
$Source=$Source.Replace($call,@'
        if(!parseOnmemoryRes_()){
            RFLiSetFileBroken(RFLiFileBrokenType_ResBroken);
            RFLiEndWorking(RFLErrcode_Loadfail);
            return RFLErrcode_Loadfail;
        }
'@)
$Source=$Source.Replace('if (fileIdx >= loader->archives[arcIdx].numFiles)', 'if (arcIdx >= RFLiArcID_Max || fileIdx >= loader->archives[arcIdx].numFiles)')
return "#include <Native/RflArchive.h>`n"+$Source
