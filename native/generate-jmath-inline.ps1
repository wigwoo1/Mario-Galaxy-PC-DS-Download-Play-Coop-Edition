$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JMath\JMath.hpp" -Raw
$hermite=@'
    const f32 span=p5-p2;
    const f32 t=(p1-p2)/span;
    const f32 t2=t*t,t3=t2*t;
    return (2*t3-3*t2+1)*p3+(t3-2*t2+t)*span*p4+(-2*t3+3*t2)*p6+(t3-t2)*span*p7;
'@
foreach($entry in @(@('inline f32 JMAFastSqrt(', '    return input > 0.0f ? std::sqrt(input) : input;'),@('inline f32 JMAHermiteInterpolation(', $hermite))){
    $start=$s.IndexOf($entry[0]);$end=$s.IndexOf('#endif',$start)
    if($start -lt 0 -or $end -lt 0){throw 'Original inline math anchor changed'}
    $s=$s.Insert($end,"#else`r`n"+$entry[1]+"`r`n")
}
foreach($count in @(3,6,12,16)){
    $start=$s.IndexOf("inline void gekko_ps_copy$count(")
    if($start -lt 0){throw "Missing paired copy $count"}
    $end=$s.IndexOf('#endif',$start)
    $dest=if($count -ge 12){'pDest'}else{'dest'}
    $src=if($count -ge 12){'pSrc'}else{'src'}
    $s=$s.Insert($end,"#else`n    if($dest!=$src)std::memcpy($dest,$src,$count*sizeof(f32));`n")
}
New-Item -ItemType Directory -Force "$n\include\JSystem\JMath"|Out-Null
Set-Content "$n\include\JSystem\JMath\JMath.hpp" ('#include <cmath>'+"`r`n"+$s) -Encoding UTF8



