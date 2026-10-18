$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\Game\Util\StringUtil.cpp" -Raw
if(!$source.Contains('#include <wstring.h>')){throw 'StringUtil legacy wide-character header anchor missing'}
$source=$source.Replace('#include <wstring.h>','#include <wchar.h>')
Set-Content "$n\src\generated\StringUtil.cpp" $source
