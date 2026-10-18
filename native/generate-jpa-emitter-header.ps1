$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=[IO.File]::ReadAllText("$p\Petari\libs\JSystem\include\JSystem\JParticle\JPAEmitter.hpp")
$anchor='s32 mpUserWork;'
if([regex]::Matches($source,[regex]::Escape($anchor)).Count-ne 1){throw 'Expected one original emitter user-work storage field.'}
if(!$source.Contains('uintptr_t getUserWork() const') -or !$source.Contains('void setUserWork(uintptr_t userWork)')){throw 'Emitter user-work accessors changed.'}
$source=$source.Replace($anchor,'uintptr_t mpUserWork; // Native host pointer; all emitter providers must share this layout.')
$source+="`nstatic_assert(sizeof(decltype(JPABaseEmitter::mpUserWork)) == sizeof(void*), `"Native emitter user-work must retain host pointers`" );`n"
$directory=Join-Path $n 'include-runtime\JSystem\JParticle';New-Item -ItemType Directory -Force -Path $directory | Out-Null
$destination=Join-Path $directory 'JPAEmitter.hpp'
if(!(Test-Path -LiteralPath $destination) -or [IO.File]::ReadAllText($destination)-ne $source){[IO.File]::WriteAllText($destination,$source,[Text.UTF8Encoding]::new($false))}
