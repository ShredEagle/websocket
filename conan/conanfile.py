from conans import ConanFile, CMake, tools

from os import path


class WebsocketConan(ConanFile):
    """ Conan recipe for Websocket """

    name = "websocket"
    license = "MIT"
    url = "https://github.com/Adnn/websocket"
    description = "Websocket implementation to mimic JS WebSocket object"
    topics = ("networking", "web")
    settings = (
        "os",
        "compiler",
        "build_type",
        "arch")
    options = {
        "shared": [True, False], # Buildhelper auto sets CMake var BUILD_SHARED_LIBS
                                 # All that is needed
        "build_tests": [True, False], # Need to manually map to CMake var BUILD_tests
    }
    default_options = {
        "shared": False,
        "build_tests": False,
    }

    requires = ("boost/[>=1.66.0]@conan/stable",)

    build_requires = ("cmake_installer/[>=3.16]@conan/stable",)

    build_policy = "missing"
    generators = "cmake_paths", "cmake"

    scm = {
        "type": "git",
        "subfolder": "cloned_repo",
        "url": "auto",
        "revision": "auto",
        "submodule": "recursive",
    }


    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_PROJECT_Websocket_INCLUDE"] = \
            path.join(self.source_folder, "cloned_repo", "cmake", "conan", "customconan.cmake")
        cmake.definitions["BUILD_tests"] = self.options.build_tests
        cmake.configure(source_folder="cloned_repo")
        return cmake


    def build(self):
        cmake = self._configure_cmake()
        cmake.build()


    def package(self):
        cmake = self._configure_cmake()
        cmake.install()


    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
