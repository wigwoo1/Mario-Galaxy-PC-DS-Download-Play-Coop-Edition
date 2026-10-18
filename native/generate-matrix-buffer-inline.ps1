$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\libs\JSystem\include\JSystem\J3DGraphAnimator\J3DMtxBuffer.hpp" -Raw
$anchor='            stfs v, 32(destination)'+"`r`n"+'        }'+"`r`n"+'#endif'
if(!$source.Contains($anchor)){throw 'Matrix buffer inline anchor changed'}
$replacement='            stfs v, 32(destination)'+"`r`n"+'        }'+"`r`n"+'#else'+"`r`n"+'        for(unsigned row=0;row<3;++row)for(unsigned col=0;col<3;++col)mpNrmMtxArr[1][mCurrentViewNo][idx][row][col]=mtx[row][col];'+"`r`n"+'#endif'
$source=$source.Replace($anchor,$replacement)
New-Item -ItemType Directory -Force "$n\include\JSystem\J3DGraphAnimator" | Out-Null
Set-Content "$n\include\JSystem\J3DGraphAnimator\J3DMtxBuffer.hpp" $source
