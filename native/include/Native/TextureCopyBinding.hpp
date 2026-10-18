#pragma once
namespace Native {
class GxCommandState;class GpuResources;
// Bind the state at the current copy point; caller must consume preceding commands
// before issuing a copy. Bound objects must outlive this serial render scope.
class TextureCopyBinding {
 const GxCommandState* previousState;GpuResources* previousResources;
public:
 TextureCopyBinding(const GxCommandState&,GpuResources&);
 ~TextureCopyBinding();
 TextureCopyBinding(const TextureCopyBinding&)=delete;
 TextureCopyBinding& operator=(const TextureCopyBinding&)=delete;
};
}
