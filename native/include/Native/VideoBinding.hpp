#pragma once
#include <Native/VideoInterface.hpp>
namespace Native {
// Stop/join the scheduler and destroy original video owners before unbinding.
class VideoBinding {
public:
 explicit VideoBinding(VideoInterface&);
 ~VideoBinding();
 VideoBinding(const VideoBinding&)=delete;
 VideoBinding& operator=(const VideoBinding&)=delete;
private:VideoInterface* previous;
};
}
