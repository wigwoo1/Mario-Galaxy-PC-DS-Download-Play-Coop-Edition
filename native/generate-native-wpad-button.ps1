$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\WPadButton.cpp" -Raw
$anchor='    if (mHold != 0) {'
if(!$s.Contains($anchor)){throw 'Missing pinned button update anchor'}
$guard=@"
    // Native device loss releases held buttons, including any repeat marker.
    if (pStatus->wpad_err == WPAD_ERR_NO_CONTROLLER) {
        mRelease = mHold & KPAD_BUTTON_MASK;
        mHold = mTrigger = mRepeat = 0;
        return;
    }

"@
$s=$s.Replace($anchor,$guard+$anchor)
Set-Content "$n\src\generated\WPadButton.cpp" $s
