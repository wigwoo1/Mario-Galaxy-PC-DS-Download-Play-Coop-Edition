#include <JSystem/JSupport/JSUInputStream.hpp>
// Base input stream owns only its status; derived classes own their resources.
JSUInputStream::~JSUInputStream() = default;

#include <JSystem/JSupport/JSUOutputStream.hpp>
// Keep an explicit body: defaulting this destructor produced a null call in the optimized Zig Windows fixture.
JSUOutputStream::~JSUOutputStream() {}


