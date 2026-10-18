# Shared by actor compilation and the original-game boot link. Save envelopes
# and per-profile chunks must use the same portable codecs on both paths.
$ErrorActionPreference='Stop'
$n=$PSScriptRoot
foreach($generator in @('generate-native-save-handler.ps1','generate-native-config-create.ps1','generate-native-config-misc.ps1','generate-native-config-mii.ps1','generate-native-sysconfig.ps1','generate-native-save-banner.ps1')){
 & (Join-Path $n $generator) | Out-Host
}
foreach($unit in @('SaveDataHandler','BinaryDataChunkHolder','ConfigDataHolder','ConfigDataMisc','ConfigDataMii','SysConfigFile','SaveDataBannerCreator')){
 $source=if($unit-eq 'BinaryDataChunkHolder'){Join-Path $n 'src\native_save_chunk_holder.cpp'}else{Join-Path $n "src\generated\$unit.cpp"}
 if(!(Test-Path -LiteralPath $source)){throw "Missing native save provider: $source"}
 [pscustomobject]@{Unit=$unit;Source=$source;OriginalObject=(Join-Path $n "build\actor-runtime-probe\$unit.obj")}
}
