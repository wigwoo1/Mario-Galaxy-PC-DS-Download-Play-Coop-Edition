#pragma once
#include <cstdint>
namespace Native {
class DepthPeekSource {
public:
 virtual ~DepthPeekSource()=default;
 virtual std::uint32_t peek(unsigned x,unsigned y)=0;
};
class DepthPeekBinding {
 DepthPeekSource* previous;
public:
 explicit DepthPeekBinding(DepthPeekSource&);
 ~DepthPeekBinding();
 DepthPeekBinding(const DepthPeekBinding&)=delete;
 DepthPeekBinding& operator=(const DepthPeekBinding&)=delete;
};
}
