@echo off

for /r %%i in (*.vert) do  (
	echo C:\VulkanSDK\1.4.313.2\Bin\glslc.exe glsl\%%~nxi -o compiled\%%~niVert.spv...
	C:\VulkanSDK\1.4.313.2\Bin\glslc.exe glsl\%%~nxi -o compiled\%%~niVert.spv
)

for /r %%i in (*.frag) do  (
	echo C:\VulkanSDK\1.4.313.2\Bin\glslc.exe glsl\%%~nxi -o compiled\%%~niFrag.spv...
	C:\VulkanSDK\1.4.313.2\Bin\glslc.exe glsl\%%~nxi -o compiled\%%~niFrag.spv
)

for /r %%i in (*.comp) do  (
	echo C:\VulkanSDK\1.4.313.2\Bin\glslc.exe glsl\%%~nxi -o compiled\%%~niComp.spv...
	C:\VulkanSDK\1.4.313.2\Bin\glslc.exe glsl\%%~nxi -o compiled\%%~niComp.spv
)
