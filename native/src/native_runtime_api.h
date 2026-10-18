#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef GALAXY_RUNTIME_EXPORTS
#define GALAXY_RUNTIME_API __declspec(dllexport)
#else
#define GALAXY_RUNTIME_API
#endif
#ifdef __cplusplus
extern "C" {
#endif
GALAXY_RUNTIME_API uint32_t galaxy_runtime_version(void);
GALAXY_RUNTIME_API const char* galaxy_runtime_error(void);
GALAXY_RUNTIME_API void* galaxy_runtime_create(int zones);
GALAXY_RUNTIME_API int galaxy_runtime_destroy(void* scene);
GALAXY_RUNTIME_API int galaxy_runtime_load_terrain(void* scene,const uint8_t* kcl,size_t kcl_size,const uint8_t* metadata,size_t metadata_size,const float matrix[12],const char* name,int zone);
GALAXY_RUNTIME_API int galaxy_runtime_tick(void* scene);
// Returns displacement and ground/wall/roof contact bits (1/2/4); input position is not mutated.
GALAXY_RUNTIME_API int galaxy_runtime_bind(void* scene,const float position[3],const float gravity[3],const float movement[3],float radius,float displacement[3],uint32_t* contacts);
#ifdef __cplusplus
}
#endif
