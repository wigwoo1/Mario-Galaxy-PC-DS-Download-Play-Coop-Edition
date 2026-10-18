#pragma once

namespace Native {
// Append one immediate XYZ position and ST texture coordinate to the active
// host GD display list using the Wii FIFO's big-endian float representation.
void writePositionTexCoord(float x, float y, float z, float s, float t);
}
