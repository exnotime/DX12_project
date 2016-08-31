solution "DX12_Project"
    configurations { "Debug", "Release" }
        flags{ "Unicode", "NoPCH" }
        libdirs { "lib" }
        includedirs { "include"}
        platforms{"x64"}

        local location_path = "solution"

        if _ACTION then
	        defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX", "AS_USE_NAMESPACE" }

            location_path = location_path .. "/" .. _ACTION
            location ( location_path )
            location_path = location_path .. "/projects"
        end

    
    configuration { "Debug" }
        defines { "DEBUG" }
        flags { "Symbols" }
        
    configuration { "Release" }
        defines { "NDEBUG", "RELEASE" }
        flags { "Optimize", "FloatFast" }

    configuration { "Debug" }
        targetdir ( "bin/" .. "/debug" )

    configuration { "Release" }
        targetdir ( "bin/" .. "/release" )


	project "DX12_Project"
		targetname "DX12_Project"
		debugdir ""
		location ( location_path )
		language "C++"
		kind "ConsoleApp"
		files { "src/**"}
		includedirs { "include/*", "src/*" }
		systemversion "10.0.14393.0"
		links { "glfw3", "d3d12", "dxgi", "d3dcompiler", "assimp"}
	configuration { "Release" }
		links {"DirectXTex", "BulletCollision", "BulletDynamics", "BulletLinearMath", "angelscript64", "as_integration"}
	configuration {"Debug"}
		links {"DirectXTexD", "BulletCollision_Debug", "BulletDynamics_Debug", "BulletLinearMath_Debug", "angelscript64d", "as_integrationD"}
      	