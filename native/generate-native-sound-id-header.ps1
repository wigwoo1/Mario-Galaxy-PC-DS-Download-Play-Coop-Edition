$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$root=Split-Path $n -Parent
$source=[IO.File]::ReadAllText((Join-Path $root 'Petari\libs\JSystem\include\JSystem\JAudio2\JAISound.hpp'))
$replacements=@(
 @(('        mID.info.type.parts.sectionID = sectionID;'+"`n"+'        mID.info.type.parts.groupID = groupID;'+"`n"+'        mID.info.waveID = waveID;'),'        mID.mComposite = ((sectionID & 255u) << 24) | ((groupID & 255u) << 16) | (waveID & 65535u);'),
 @('return mID.info.type.parts.sectionID;','return static_cast<u8>(mID.mComposite >> 24);'),
 @('mID.info.type.parts.sectionID = id;','mID.mComposite = (mID.mComposite & 0x00ffffffu) | (u32(id) << 24);'),
 @('return mID.info.type.parts.groupID;','return static_cast<u8>(mID.mComposite >> 16);'),
 @('mID.info.type.parts.groupID = id;','mID.mComposite = (mID.mComposite & 0xff00ffffu) | (u32(id) << 16);'),
 @('return mID.info.waveID;','return static_cast<u16>(mID.mComposite);'),
 @('mID.info.waveID = id;','mID.mComposite = (mID.mComposite & 0xffff0000u) | u32(id);')
)
$source=$source.Replace("`r`n","`n")
foreach($pair in $replacements){if([regex]::Matches($source,[regex]::Escape($pair[0])).Count-ne 1){throw "Original sound-ID anchor changed: $($pair[0])"};$source=$source.Replace($pair[0],$pair[1])}
# Preserve the public split-field aliases used by original JAISeMgr as well.
$infoPattern='(?s)        struct \{\n            union \{\n                u16 value;\n                struct \{\n                    u8 sectionID;\n                    u8 groupID;\n                \} parts;\n            \} type;\n            u16 waveID;\n        \} info;'
$nativeInfo=@"
        struct {
            u16 waveID;
            union {
                u16 value;
                struct {
                    u8 groupID;
                    u8 sectionID;
                } parts;
            } type;
        } info;
"@
if([regex]::Matches($source,$infoPattern).Count-ne 1){throw 'Original sound-ID public split fields changed'}
$source=[regex]::Replace($source,$infoPattern,$nativeInfo)
$source+="`n// IDs keep their Wii numeric value on native little-endian hosts.`nstatic_assert(sizeof(JAISoundID) == sizeof(u32), `"Sound ID storage must remain four bytes`" );`n"
$directory=Join-Path $n 'include-runtime\JSystem\JAudio2';New-Item -ItemType Directory -Force -Path $directory|Out-Null
$destination=Join-Path $directory 'JAISound.hpp'
if(!(Test-Path -LiteralPath $destination)-or [IO.File]::ReadAllText($destination)-ne $source){[IO.File]::WriteAllText($destination,$source,[Text.UTF8Encoding]::new($false))}