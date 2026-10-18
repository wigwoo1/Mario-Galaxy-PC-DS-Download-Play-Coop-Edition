$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Enemy\BegomanBase.cpp" -Raw
$s=$s.Replace('_C0.setEulerDegree(mRotation.x, mRotation.y, mRotation.z);','_C0.setEuler(mRotation.x * PI_180, mRotation.y * PI_180, mRotation.z * PI_180);').Replace('yRotationQuat.setRotate(0.0f, 1.0f, 0.0f, mRotation.y);','yRotationQuat.setRotate(TVec3f(0.0f, 1.0f, 0.0f), mRotation.y);').Replace('PSQUATMultiply(&yRotationQuat, &_D0, &_D0);','PSQUATMultiply(reinterpret_cast<const Quaternion*>(&yRotationQuat), reinterpret_cast<const Quaternion*>(&_D0), reinterpret_cast<Quaternion*>(&_D0));').Replace('MR::getWaterAreaObj(&info, mPosition) != nullptr','MR::getWaterAreaObj(&info, mPosition)')
Set-Content "$n\src\generated\BegomanBase.cpp" $s
