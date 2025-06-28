# RealEngine
Refactorable and multipurpose realtime 3D engine made in Vulkan

# Planned Next Steps

- [x] Hello Triangle
- [ ] Read from .s72 files
- [ ] Combine vertex and index buffers
    - "Driver developers recommend that you also store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers"
    - resource: https://developer.nvidia.com/vulkan-memory-management
- [ ] Allow for different vertex types within same scene ?
- [ ] Mesh culling
- [ ] Add spline keyframing to animation system
- [ ] Import animation / basic materials from Blender
- [ ] Compute-based cluster culling 
    - remember to use CLUSTER_SHADER_HUWAI bit and (maybe) HOST_CAHCED_BIT 
- [ ] Basic shadow mapping 
- [ ] Advanced shadow mapping? (varince shadow mapping, cascade shadow mapping)
- [ ] More thourghtful memeory allocation - custom allocator of library: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
- [ ] Mesh stripification
- [ ] Linear triangle cache ?
    - resource: https://tomforsyth1000.github.io/papers/fast_vert_cache_opt.html
- [ ] Deferred / forward+ rendering
- [ ] Tonemapping
- [ ] Multithreading (particilarly drawcalls, shadow mapping, etc)
-shadow mapping of next frame in flight of deferred shading of cur frame?
       	since shadow mapping usually doesn't use shader cores?
    - mesh cullling / scene tree descending
    - drawing if not doing deferred (ie not one DrawIndirect call)
- [ ] Custom vector / matrix types with vectorized instructions?
- [ ] Tesselation / mesh shaders?
- [ ] Full skinned mesh / inverse kinematics animation
    - probably need seperate sections of vertex buffer (and new render pass? new buffer?) for nonstatic vertex data
- [ ] Rope / spring-mass / softbody simulation

