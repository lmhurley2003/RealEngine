# RealEngine
Refactorable and multipurpose realtime 3D engine made in Vulkan

# Planned Next Steps

- [x] Hello Triangle
- [ ] Reconfigure to use CMake and CMRC
- [ ] Read from .s72 files, save scene / program state in binary for fast reload
- [ ] Combine vertex and index buffers
    - "Driver developers recommend that you also store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers"
    - resource: https://developer.nvidia.com/vulkan-memory-management
- [ ] Allow for different vertex types within same scene ?
- [ ] Mesh culling
- [ ] Acceleration structure(s) (BVH, BLAS thingy)
   - maybe seperate or no acceleration structure for animated geometry
- [ ] Add spline keyframing to animation system
- [ ] Import animation / basic material data from Blender
- [ ] Pipeline caching
- [ ] Texture atlases / megatexture / virtual textures \
 Resources : 
    -  https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/
    -  https://www.mrelusive.com/publications/papers/Software-Virtual-Textures.pdf
    -  https://web.archive.org/web/20190204083214/http://s09.idav.ucdavis.edu/talks/05-JP_id_Tech_5_Challenges.pdf
- [ ] Compute-based cluster culling 
    - remember to use CLUSTER_SHADER_HUWAI bit and (maybe) HOST_CAHCED_BIT 
- [ ] Basic shadow mapping 
- [ ] Font loader both with ray marched renderer and font atlas 
- [ ] Advanced shadow mapping? (variance shadow mapping, cascade shadow mapping)
- [ ] More thoughtful memory allocation - custom allocator or library: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
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

# Game specific abstractions / technical musings
- [ ] Probably need abstraction for creating seperate rendering layers like in Unity
    - Useful for creating several "Windows" 
    - Each window would need seperate depth buffer, can do in one draw call ? Use subpasses ?
    - Probably use stencil buffer rather than depth buffer
    - Actually nvm proably use multiple draw calls and just reset depth buffer, but does mean needing
    to manually order "windows" back to front (but that's prbably pretty trivial)
        -Could this work with occlusion culling ? Maybe occlusion culling would be faster 
         CPU-side since we probably have a small, fixed-size number of screen-space aligned occluders
    -Maybe actually use extent feature ?
    - The main question I am havving is that the most obvious solution is to have each window render to an offscreen image buffer,
    then place that image buffer on the screen. But there's something that feels sort fo icky about that, and feels like I could instead 
    just render things striaght to screen with a throughtful use of depth clearing, passing in offset of window with push constants, and/or
    using dynamic extent
- [ ] Maybe release each seperate "program" on Itch.io, combine in final game with meta layer
    - If I want to run games in browser, proabably need abstraction to use different grpahics APIs ie WebGL
- [ ] Probably should end up pre redering SOME stuff (ie pinball background), but want to prerender as little as \
 possible just from a stand point of creative extensibility (subverting medium expectations), like with the \
"prerendered" background that ends up moving
- [ ] Text box class with seperate renderes (ie bitmap/SDF or raymarched)
- [ ] How to make lower res ? render to offsceren texture, upscale ? IN general, if I want to make thigns lower
res, I actually want to benefit from saved performance cost
- [ ] Do I want 4:3 aspect ratio ? Help with general vibe, but may make things harder / less
aesthetically pleasing in long run
- [ ] Overarching tonemapping step for crt-line shader ? A part of me thinks this is copying 
Animal Well too much but idk maybe I have to get rid of this aversion to copying anything
- [ ] Each mesh/renderable objet probably needs a tag for which window it is a part of
- [ ] Look into entity systems
- [ ] Try to make  everything a single executable (ie no shader library)
    - CMRC looks like a good option : https://github.com/vector-of-bool/cmrc  
- [ ] Have a "clickable" / "interactable" class with own BVH / quadtree structure ? 
- [ ] Porbably need diff view/proj matrix per window
- [ ] Open question : just sort draw calls back to front or use depth buffer with orthogonal camera matrix 
 to order windows ? 
- [ ] Probably move mainLoop out of App class into general Game class ?
- [ ] deferred deferred rendering is probably best since lots of visualys will be occluded by windows
- [ ] Should lights be in seperate descriptor set per program ? or use push constants to have an offset+size approach ?
    -On second thought almost certainly better to do offset+size approach since the "set=n" directive is constant, would need
     separate shaders per window
    -But maybe have seperate sets for static vs dynamic lights ?
- [ ] should probably make a UI Window / clickable class that can serve as a tree with other clickable images within it.
    -Things within windows will be mutually exlcusive with other clickable things, and will not overlap with other windows,
-   so implicitly creates an acceleration tree-like structure for speeding up click hits/misses
- [ ] Maybe instead of doing pinball as first test, do desktop backgroun (grass instancing, dither, etc)
- [ ] If I want a true "oh wow the scope of this game is huge" moment, make this desktop bg world HUGE, like
maybe hide a whole small-scale open world survival minigame behind it

# Game design princiciples
- Maybe overacrching game loop is pinball, with SickPix Pro !, email, Sims-like game, screensavers as meta elements
    - Beginning idea was to make whatever the job is the gameplay loop -- how to gameify writing emails and photoshop? 
- Sort of want everything to have an "opening loading animation", but need to figure out whether that \
animation will actually be loading things in background or is just aesthetic 
    -  Should that be a puzzle ? doing something as a background "loads" ? 
- Maybe part of puzzle structure is making "shortcuts" of program-contextual object onto desktop
     - IE create short cut for "locked chest" from email and short cut for "crow bar" from pinball, drag \
      on icon onto another to open -- similar to combining things in point-and-click games
     - Minor idea, but maybe icons also have normal maps to react to lighting in background
- Having seperate, discrete puzzle "layers" like in Animal Well
- Purposefully anachronistic, with both 
- Have moment where grpahics "breaksdown", ie first shuttling shadow mapping from varince to normal to none,
lowering multisampling samples, in general purposfilly alaisging stuff
- Maybe have icons interact with desktop background ?
- Aesthetics-wise, probably would feel more realistic to have some programs have different resolutions than main
program resolution. 
- Add a lot of small interactive touches. Like think about Pajama Sam, there's not a lot of complex gamepley yet the
players are propelled by small moments of interactivty and humor
- 

#V VMA allocator notes
- all allocations made from larger VkDevice Memory allocation 
- can make dedicated allocation block with VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, but
library will do this automatically under certain circumstances (ie )

# Pinball
- [ ] Likely a good start for a proof of concept
- [ ] Space cadet has multiple lights, likely prerenderered, can I get away with using just bloom instead of
Doom-esque forward+ light rendering ?
- [ ] Maybe use walk mesh to describe bounds, instead of creating full physics engine
- [ ] Drag pinball paddles + ball into desktop, use ball to "break" pinball icon
    - Make breakable objects in pinball game with distinct material to make puzzle concrete and "gettable"
# Minor optimization options
- [ ] use if constexpr instead of if to make sure msvc discards branches
- [ ] unroll constant maps into if constexpr / switch case branches
- [ ] use string hashing to use switch branches for string
- [x] Use different compiler/linker than msvc or at least program other than Visual Studio for
final build -- including unused header files increases .exe size, which doesn't seemt to be true for
other compilers
    - DONE : using clang, decreased .exe size by 3x ???    

# Resources to look at
- [ ] Handmade Hero
    - Hot reloading
    - Using DLLs
    - C-style programming
- [ ] Screenspace ambient occlusion https://research.nvidia.com/sites/default/files/pubs/2012-06_Scalable-Ambient-Obscurance/McGuire12SAO.pdf
- [ ] Dependency graphs, buffer management, multithreading practices https://www.youtube.com/watch?v=mdPeXJ0eiGc&t=902s
- [x] Billy Basso interview  : https://youtu.be/YngwUu4bXR4?si=yCt0EJOj-UvZdhzP
    - Use MIDI to tweak variables ?
