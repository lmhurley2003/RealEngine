@echo off

for /r %%i in (*.vert) do  (
	echo C:\VulkanSDK\1.4.313.2\Bin\glslc.exe %%~nxi -o %%~niVert.spv...
	C:\VulkanSDK\1.4.313.2\Bin\glslc.exe %%~nxi -o %%~niVert.spv
)

for /r %%i in (*.frag) do  (
	echo C:\VulkanSDK\1.4.313.2\Bin\glslc.exe %%~nxi -o %%~niFrag.spv...
	C:\VulkanSDK\1.4.313.2\Bin\glslc.exe %%~nxi -o %%~niFrag.spv
)

for /r %%i in (.*comp) do  (
	echo C:\VulkanSDK\1.4.313.2\Bin\glslc.exe %%~nxi -o %%~niComp.spv...
	C:\VulkanSDK\1.4.313.2\Bin\glslc.exe %%~nxi -o %%~niComp.spv
)

pause
