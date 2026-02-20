from conan import ConanFile
from conan.tools.cmake import cmake_layout,CMake

class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    recipe_folder="./build"
    def requirements(self):
        if self.requires is None:
            exit(1)
        self.requires("fmt/12.1.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        pass

    def layout(self):
        self.folders.build = "build"
        self.folders.generators = "build"