$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\TalkUtil.cpp" -Raw
$out=$s.Substring(0,$s.IndexOf('namespace MR {'))+"namespace MR {`r`n"
foreach($sig in @('    bool tryTalkForce(TalkMessageCtrl* pCtrl)', '    const MtxPtr getMessageBalloonFollowMatrix(', '    const TVec3f& getMessageBalloonFollowOffset(', '    bool isTalkTalking(', '    bool isShortTalk(', '    bool isNearPlayer(const TalkMessageCtrl* pCtrl, f32 dist)', '    TalkMessageCtrl* createTalkCtrl(LiveActor*', '    bool tryTalkForceWithoutDemoMarioPuppetableAtEnd(', '    void registerBranchFunc(', '    void registerEventFunc(', '    void registerAnimeFunc(', '    void registerKillFunc(', '    void setMessageArg(TalkMessageCtrl* pCtrl, int value)', '    void resetNode(TalkMessageCtrl* pCtrl)', '    void forwardNode(TalkMessageCtrl* pCtrl)', '    bool tryForwardNode(TalkMessageCtrl* pCtrl)', '    bool isExistNextNode(const TalkMessageCtrl* pCtrl)', '    void setDistanceToTalk(TalkMessageCtrl* pCtrl, f32 dist)', '    bool tryTalkNearPlayer(TalkMessageCtrl* pCtrl)', '    bool tryTalkNearPlayerAtEnd(TalkMessageCtrl* pCtrl)', '    bool tryTalkForceAtEnd(TalkMessageCtrl* pCtrl)', '    bool tryTalkForceWithoutDemo(TalkMessageCtrl* pCtrl)', '    bool tryTalkForceWithoutDemoAtEnd(TalkMessageCtrl* pCtrl)', '    bool tryTalkSelectLeft(TalkMessageCtrl* pCtrl)', '    TalkMessageCtrl* createTalkCtrlDirect(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID, const TVec3f& rOffset, MtxPtr pMtx)', '    TalkMessageCtrl* createTalkCtrlDirectOnRootNodeAutomatic(LiveActor* pActor, const JMapInfoIter& rIter, const char* pMessageID,', '    void setMessageBalloonFollowOffset(TalkMessageCtrl* pCtrl, const TVec3f& rOffs)', '    void onRootNodeAutomatic(TalkMessageCtrl* pCtrl)')){
$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing dialogue signature $sig"};$b=$s.IndexOf('{',$a);$d=1;$e=$b+1;while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++};$out+=$s.Substring($a,$e-$a)+"`r`n"
}
$out+="}`r`n";Set-Content "$n\src\generated\RuntimeTalkQueries.cpp" $out






