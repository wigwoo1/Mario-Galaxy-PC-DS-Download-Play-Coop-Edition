param()
$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$inputPath="$n\src\generated\CollisionPartsQuery.cpp"
$outputPath="$n\src\generated\CollisionPartsQueryNoParts.cpp"
$text=[IO.File]::ReadAllText($inputPath)
$signatures=@(
 'CollisionParts::CollisionParts()',
 'CollisionParts::checkStrikeLine(',
 'CollisionParts::checkStrikeBall(',
 'CollisionParts::checkStrikeBallCore(',
 'CollisionParts::calcCollidePosition(',
 'CollisionParts::projectToPlane(',
 'CollisionParts::updateMtx(',
 'CollisionParts::makeEqualScale(',
 'CollisionParts::setMtx(',
 'CollisionParts::updateBoundingSphereRange(',
 'CollisionParts::updateBoundingSphereRangePrivate('
)
$ranges=[Collections.Generic.List[object]]::new()
foreach($signature in $signatures){
 $matches=[regex]::Matches($text,[regex]::Escape($signature))
 if($matches.Count -lt 1){throw "Expected generated query definition for $signature"}
 foreach($match in $matches){
  $i=$match.Index
  $start=$text.LastIndexOf("`n",$i); if($start -lt 0){$start=0}else{$start++}
  $open=$text.IndexOf('{',$i);if($open -lt 0){throw "Missing body for $signature"}
  $depth=0;$end=$open
  while($end -lt $text.Length){if($text[$end] -eq '{'){$depth++};if($text[$end] -eq '}'){$depth--};$end++;if($depth -eq 0){break}}
  if($depth -ne 0){throw "Unbalanced function body for $signature"}
  while($end -lt $text.Length -and ($text[$end] -eq "`r" -or $text[$end] -eq "`n")){$end++}
  $ranges.Add([pscustomobject]@{Start=$start;End=$end;Signature=$signature})
 }
}
foreach($range in ($ranges | Sort-Object Start -Descending)){$text=$text.Remove($range.Start,$range.End-$range.Start)}
foreach($signature in $signatures){if($text.Contains($signature)){throw "Removal failed for $signature"}}
foreach($signature in @('Binder::clear()','Binder::Binder(','Triangle::getNormal(')){if(!$text.Contains($signature)){throw "Shared collision helper lost: $signature"}}
[IO.File]::WriteAllText($outputPath,$text)
Write-Output "Preserved query helpers; removed $($ranges.Count) overlapping CollisionParts methods."