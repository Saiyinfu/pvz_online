add_requires("vcpkg::imgui[dx9-binding,win32-binding]", "vcpkg::polyhook2", "vcpkg::zydis", "vcpkg::ixwebsocket")

target("clientDll")
    set_kind("shared")
    add_files("clientDll/*.cpp")
    set_languages("cxx17")
    add_linkdirs("clientDll")
    add_packages("vcpkg::imgui[dx9-binding,win32-binding]", "vcpkg::polyhook2", "vcpkg::zydis", "vcpkg::ixwebsocket")

target("launcher")
    set_kind("binary")
    add_rules("win.sdk.application")
    add_options("wchar")
    add_files("launcher/*.cpp")
    add_files("launcher/*.rc")
    add_defines("UNICODE", "_UNICODE")
    add_packages("vcpkg::imgui[dx9-binding,win32-binding]")

    on_run(
        function(target)
        os.run("")
        end
    )
