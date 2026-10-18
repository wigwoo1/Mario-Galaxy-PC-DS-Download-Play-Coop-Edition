$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\WPadStick.cpp" -Raw
$anchor='    if (!MR::isDeviceFreeStyle(pStatus)) {'
if(!$s.Contains($anchor)){throw 'Missing original stick update anchor'}
$guard=@"
    if (pStatus != nullptr && pStatus->wpad_err == WPAD_ERR_NO_CONTROLLER) {
        mRelease = mHold;
        mHold = mTrigger = 0;
        mSpeed = mStick.distance(TVec2f(0.0f, 0.0f));
        mStick.set(0.0f, 0.0f);
        mIsTriggerUp = mIsTriggerDown = mIsHoldUp = mIsHoldDown = false;
        return;
    }

"@
$s=$s.Replace($anchor,$guard+$anchor)
Set-Content "$n\src\generated\WPadStick.cpp" $s
