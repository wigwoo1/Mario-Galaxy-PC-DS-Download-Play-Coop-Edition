$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=[System.IO.File]::ReadAllText("$p\Petari\src\Game\Player\MarioActor.cpp").Replace("`r`n","`n")
$open="switch (action) {`n    case 1:`n        bool didSpinPunch = true;"
if(([regex]::Matches($source,[regex]::Escape($open))).Count -ne 1){throw 'MarioActor spin-action opening anchor changed'}
$source=$source.Replace($open,"switch (action) {`n    case 1: {`n        bool didSpinPunch = true;")
$start=$source.IndexOf('switch (action) {')
$close="        break;`n    case 2:"
$closeAt=$source.IndexOf($close,$start)
if($closeAt -lt 0){throw 'MarioActor spin-action closing anchor missing'}
$source=$source.Remove($closeAt,$close.Length).Insert($closeAt,"        break;`n    }`n    case 2:")
$case4="case 4:`n        if (mMario->_418 != 0) {"
if(([regex]::Matches($source,[regex]::Escape($case4))).Count -ne 1){throw 'MarioActor Teresa case opening anchor changed'}
$source=$source.Replace($case4,"case 4: {`n        if (mMario->_418 != 0) {")
$case4Close="        _946 = pConstants->mTeresaWallThroughTime + pConstants->mSpinIntervalTime;`n        break;`n    case 5:"
if(([regex]::Matches($source,[regex]::Escape($case4Close))).Count -ne 1){throw 'MarioActor Teresa case close anchor changed'}
$source=$source.Replace($case4Close,"        _946 = pConstants->mTeresaWallThroughTime + pConstants->mSpinIntervalTime;`n        break;`n    }`n    case 5:")
[System.IO.File]::WriteAllText("$n\src\generated\MarioActor.cpp",$source,[System.Text.UTF8Encoding]::new($false))
