#pragma once
#include <cstddef>
#include <cstdint>
namespace Native {
class CommandSubmitter {
public:
 virtual ~CommandSubmitter()=default;
 // Copy or consume bytes before returning. Throw only before accepting ownership;
 // accepted GPU execution failures must be reported through waitForFinish.
 virtual void submit(const std::uint8_t*,std::size_t,bool finish)=0;
 virtual void waitForFinish()=0;
 // Requests cooperative cancellation of the current host frame. Implementations
 // must not touch a thread-affine graphics context from the caller.
 virtual void abortFrame() noexcept {}
};
// True after a host command batch has been accepted and until its finish fence
// completes. Used by the GX status adapter in place of Wii FIFO status bits.
bool gxCommandCompletionPending() noexcept;
void waitForGxAbortRecovery();
void markGxAbortRecoveryReady() noexcept;
void appendGxAbortResetCommand();
class CommandSubmissionBinding {
public:
 explicit CommandSubmissionBinding(CommandSubmitter&);
 ~CommandSubmissionBinding();
 CommandSubmissionBinding(const CommandSubmissionBinding&)=delete;
 CommandSubmissionBinding& operator=(const CommandSubmissionBinding&)=delete;
private:CommandSubmitter* previous;
};
}

