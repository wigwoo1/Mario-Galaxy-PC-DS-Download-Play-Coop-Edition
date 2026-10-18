$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$package=Get-Content -LiteralPath (Join-Path $root 'tools/zig-package.json') -Raw | ConvertFrom-Json
$compiler=Join-Path $root 'tools/zig-x86_64-windows-0.16.0/zig.exe'
if(!(Test-Path -LiteralPath $compiler)){
 $download=Join-Path $root 'tools/zig.zip'
 Invoke-WebRequest -Uri $package.tarball -OutFile $download
 if((Get-FileHash -LiteralPath $download).Hash-ne $package.shasum){throw 'Zig checksum mismatch.'}
 Expand-Archive -LiteralPath $download -DestinationPath (Join-Path $root 'tools') -Force
}
$build=Join-Path $root 'native/build'
New-Item -ItemType Directory -Force -Path $build | Out-Null
Copy-Item -LiteralPath (Join-Path $root 'native/support/crash-trace.cpp') -Destination (Join-Path $build 'crash-trace.cpp')
$reference=Join-Path $root 'external/dolphin-dsp-reference'
if(!(Test-Path -LiteralPath $reference)){
 New-Item -ItemType Directory -Force -Path (Split-Path $reference -Parent) | Out-Null
 git clone --filter=blob:none https://github.com/dolphin-emu/dolphin.git $reference
 if($LASTEXITCODE){throw 'Dolphin reference clone failed.'}
 git -C $reference checkout e2ad09306e6d371f7272205bdc363da9fe05fe08
 if($LASTEXITCODE){throw 'Dolphin reference checkout failed.'}
}
Write-Output 'Dependencies prepared. Clean-checkout full boot reproduction remains unverified; see BUILDING.md.'
