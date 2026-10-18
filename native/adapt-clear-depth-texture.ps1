param([Parameter(Mandatory)][string]$SourceText)
$ErrorActionPreference='Stop'
$marker='nativeClearDepthTextureAddress'
if($SourceText.Contains($marker)){return $SourceText}
$anchor='    GXInitTexObj(&clear_z_tobj, clearZTexData, 4, 4, GX_TF_Z24X8, GX_REPEAT, GX_REPEAT, GX_FALSE);'
if([regex]::Matches($SourceText,[regex]::Escape($anchor)).Count-ne 1){throw 'Original static clear-depth texture initializer changed.'}
$registration=@'
    // Authored u32 words describe big-endian GX texels. Keep CPU words intact
    // and register a native-word GPU snapshot for this module-lifetime texture.
    static const u32 nativeClearDepthTextureAddress = Native::gpuResources().addNativeWords(clearZTexData, sizeof(clearZTexData));
    (void)nativeClearDepthTextureAddress;
'@
return '#include <Native/GpuResources.hpp>'+"`n"+$SourceText.Replace($anchor,$registration+"`n"+$anchor)
