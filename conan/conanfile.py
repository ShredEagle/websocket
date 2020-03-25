from conans import ConanFile, CMake, tools

from os import path


class WebsocketConan(ConanFile):
    """ Conan recipe for Websocket """

    name = "websocket"
    license = "MIT"
    url = "https://github.com/Adnn/websocket"
    description = "Websocket implementation to mimic JS WebSocket object"
    topics = ("networking", "web")
    settings = ("os", "compiler", "build_type", "arch")
    options = {
        "shared": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "shared": False,
        "build_tests": False,
    }

    requires = ("boost/1.72.0",)

    build_requires = ("cmake_installer/[>=3.16]@conan/stable",)

    build_policy = "missing"
    generators = "cmake_paths", "cmake"

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto",
        "submodule": "recursive",
    }


    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["CMAKE_PROJECT_Websocket_INCLUDE"] = \
            path.join(self.source_folder, "cmake", "conan", "customconan.cmake")
        cmake.definitions["BUILD_tests"] = self.options.build_tests
        cmake.configure()
        return cmake


    def build(self):
        cmake = self._configure_cmake()
        cmake.build()


    def package(self):
        cmake = self._configure_cmake()
        cmake.install()


    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
