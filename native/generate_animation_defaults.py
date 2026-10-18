from pathlib import Path
import re
root = Path(__file__).resolve().parent
source = (root.parent / 'Petari/src/Game/Player/MarioConst.cpp').read_text(encoding='utf-8')
body = source.split('static MarioConstTable gMarioConstDefault = {', 1)[1].split('};', 1)[0]
fields = {'mWeightBlendRatio':'walk', 'mWeightBlendRatioSwim':'swim', 'mSliderTiltRatio':'slide', 'mWalkSpeed':'speedConstant', 'mStartSpinAnimeRatio':'startRatio', 'mSlopeAnimeRatio':'slopeRatio', 'mSlopeSpinAnimeRatio':'spinRatio'}
fields.update({name:name for name in ('mInertiaStandardStop','mInertiaStandardMax','mInertiaStartSpin','mInertiaStop','mInertiaIceStandardStop','mInertiaIceStandardMax','mInertiaIceStartSpin','mInertiaIceStop','mInertiaSlipStandardStop','mInertiaSlipStandardMax','mInertiaSlipStartSpin','mInertiaSlipStop','mInertiaTurnSlip','mInertiaTurning','mInertiaJumpFinish','mInertiaSquat','mInertiaTornadoBrake','mInertiaTornadoAccel','mInertiaReflectSlip','mInertiaOverSpeed')})
lines = ['#pragma once', '// Generated from Petari MarioConst.cpp gMarioConstDefault; runtime overrides pending.', 'namespace NativeMarioAnimationDefaults {']
for field, name in fields.items():
    matches = re.findall(r'/\*\s*' + field + r'\s*\*/\s*([0-9.]+f)\s*,', body)
    if len(matches) != 1:
        raise ValueError(f'Missing or ambiguous constant: {field}')
    value = float(matches[0][:-1])
    limit = 100 if field == 'mWalkSpeed' else 16 if 'AnimeRatio' in field else 1
    if not 0 <= value <= limit:
        raise ValueError(f'Invalid ratio: {field}')
    lines.append(f'inline constexpr float {name} = {matches[0]};')
slow = re.findall(r'/\*\s*mSlowStartTime\s*\*/\s*(\d+)\s*,', body)
if len(slow) != 1 or not 0 <= int(slow[0]) <= 600:
    raise ValueError('Invalid slow-start duration')
lines.append(f'inline constexpr unsigned slowStart = {slow[0]};')
spin = re.findall(r'/\*\s*mStartSpinTime\s*\*/\s*(\d+)\s*,', body)
if len(spin) != 1 or not 0 <= int(spin[0]) <= 600:
    raise ValueError('Invalid start-spin duration')
lines.append(f'inline constexpr unsigned startSpinTime = {spin[0]};')
lines.append('}')
(root / 'src/mario_animation_defaults.hpp').write_text('\n'.join(lines)+'\n', encoding='utf-8')
print('Generated Mario animation default ratios from original source')






