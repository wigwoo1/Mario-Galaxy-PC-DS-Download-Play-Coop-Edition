$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\libs\JSystem\include\JSystem\J3DGraphBase\J3DDrawBuffer.hpp" -Raw
$anchor="    return out;`r`n#endif"
if(!$source.Contains($anchor)){$anchor="    return out;`n#endif"}
if(!$source.Contains($anchor)){throw 'Draw buffer depth helper anchor missing'}
$source=$source.Replace($anchor,"    return out;`r`n#else`r`n    return m[2][0]*v.x+m[2][1]*v.y+m[2][2]*v.z+m[2][3];`r`n#endif")
New-Item -ItemType Directory -Force "$n\include\JSystem\J3DGraphBase" | Out-Null
Set-Content "$n\include\JSystem\J3DGraphBase\J3DDrawBuffer.hpp" $source
