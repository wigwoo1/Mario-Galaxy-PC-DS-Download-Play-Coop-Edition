# Inventory unresolved COFF externals across only successful native Mario objects.
# This is an integration report, not a linker or evidence of runnable gameplay.
param([string]$ProbeDirectory=(Join-Path $PSScriptRoot 'build\mario-runtime-all-player'))
$ErrorActionPreference='Stop'
$results=Get-Content -LiteralPath (Join-Path $ProbeDirectory 'results.json') -Raw | ConvertFrom-Json
$definitions=[Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
$references=[Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
$objects=0
foreach($result in $results) {
    if($result.ExitCode -ne 0){continue}
    $path=Join-Path $ProbeDirectory ($result.Unit+'.obj')
    $bytes=[IO.File]::ReadAllBytes($path)
    if($bytes.Length -lt 20 -or [BitConverter]::ToUInt16($bytes,0) -ne 0x8664){throw "Unsupported COFF object: $path"}
    [long]$table=[BitConverter]::ToUInt32($bytes,8)
    [long]$count=[BitConverter]::ToUInt32($bytes,12)
    [long]$strings=$table+18*$count
    if($strings+4 -gt $bytes.Length){throw "Invalid symbol table: $path"}
    [long]$stringLength=[BitConverter]::ToUInt32($bytes,$strings)
    if($stringLength -lt 4 -or $strings+$stringLength -gt $bytes.Length){throw "Invalid string table: $path"}
    for([long]$i=0;$i -lt $count;) {
        [long]$offset=$table+18*$i
        $section=[BitConverter]::ToInt16($bytes,$offset+12)
        $value=[BitConverter]::ToUInt32($bytes,$offset+8)
        $storage=$bytes[$offset+16]
        $aux=$bytes[$offset+17]
        if($i+1+$aux -gt $count){throw "Invalid auxiliary records: $path"}
        if($storage -eq 2) {
            if([BitConverter]::ToUInt32($bytes,$offset) -eq 0) {
                [long]$relative=[BitConverter]::ToUInt32($bytes,$offset+4)
                if($relative -lt 4 -or $relative -ge $stringLength){throw "Invalid symbol name: $path"}
                $start=$strings+$relative; $end=$start
                while($end -lt $strings+$stringLength -and $bytes[$end] -ne 0){$end++}
                if($end -eq $strings+$stringLength){throw "Unterminated symbol name: $path"}
            } else {
                $start=$offset; $end=$start
                while($end -lt $offset+8 -and $bytes[$end] -ne 0){$end++}
            }
            $name=[Text.Encoding]::UTF8.GetString($bytes,$start,$end-$start)
            if($section -gt 0 -or ($section -eq 0 -and $value -gt 0)) {
                if(-not $definitions.ContainsKey($name)){$definitions[$name]=@()}
                $definitions[$name]+=$result.Unit
            } elseif($section -eq 0 -and $value -eq 0) {
                if(-not $references.ContainsKey($name)){$references[$name]=@()}
                $references[$name]+=$result.Unit
            }
        }
        $i+=1+$aux
    }
    $objects++
}
$unresolved=@($references.Keys | Where-Object {-not $definitions.ContainsKey($_)} | Sort-Object | ForEach-Object {
    [pscustomobject]@{Symbol=$_;ReferencedBy=@($references[$_] | Sort-Object -Unique)}
})
$report=[pscustomobject]@{Scope=$Scope;Objects=$objects;DefinedSymbols=$definitions.Count;UnresolvedSymbols=$unresolved.Count;Unresolved=$unresolved}
$report | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $ProbeDirectory 'link-dependencies.json') -Encoding UTF8
Write-Output "Objects=$objects Defined=$($definitions.Count) Unresolved=$($unresolved.Count)"

