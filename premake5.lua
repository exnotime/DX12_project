solution "Exjobb_GPU_Culling"
    configurations { "Debug", "Release", "Dev" }
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
        flags { "OptimizeSpeed", "FloatFast" }

    configuration { "Dev" }
        defines { "NDEBUG", "DEV" }
        flags { "OptimizeSpeed", "FloatFast", "Symbols" }

    configuration { "Debug" }
        targetdir ( "bin/" .. "/debug" )

    configuration { "Release" }
        targetdir ( "bin/" .. "/release" )

        configuration { "Dev" }
        targetdir ( "bin/" .. "/dev" )


	project "Exjobb_Engine"
		targetname "Engine"
		debugdir ""
		location ( location_path )
		language "C++"
		kind "ConsoleApp"
		files { "src/**"}
		includedirs { "include/", "src/" }
		systemversion "10.0.14393.0"
		links { "glfw3", "d3d12", "dxgi", "d3dcompiler", "assimp", "amd_ags_x64", "nvapi64" }
	configuration { "Release" }
		links {"DirectXTex", "BulletCollision", "BulletDynamics", "BulletLinearMath", "angelscript64", "as_integration"}
    configuration { "Dev" }
        links {"DirectXTex", "BulletCollision", "BulletDynamics", "BulletLinearMath", "angelscript64", "as_integration"}
	configuration {"Debug"}
		links {"DirectXTexD", "BulletCollision_Debug", "BulletDynamics_Debug", "BulletLinearMath_Debug", "angelscript64d", "as_integrationD"}
      	