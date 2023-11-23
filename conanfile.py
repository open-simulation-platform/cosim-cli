import os

from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain
from conan.tools.env import VirtualBuildEnv
from conan.tools.files import copy

class CosimCLIConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    default_options = { "*:shared": True }

    def requirements(self):
        self.tool_requires("cmake/[>=3.19]")
        if self.settings.os == "Linux":
            self.tool_requires("patchelf/[>=0.18]")
        self.requires("libcosim/0.11.0@osp/testing-feature_conan-2")
        self.requires("boost/[>=1.71]")

    def generate(self):
        # Import shared libraries and executables from dependency packages
        # to the 'dist/' folder.
        bindir = os.path.join(self.build_folder, "dist", "bin")
        dldir = (bindir if self.settings.os == "Windows" else
            os.path.join(self.build_folder, "dist", "lib"))
        dependency_libs = {
            # For some dependencies, we only want a subset of the libraries
            "boost" : [
                "boost_atomic*",
                "boost_chrono*",
                "boost_container*",
                "boost_context*",
                "boost_date_time*",
                "boost_filesystem*",
                "boost_locale*",
                "boost_log*",
                "boost_log_setup*",
                "boost_program_options*",
                "boost_random*",
                "boost_regex*",
                "boost_serialization*",
                "boost_system*",
                "boost_thread*"],
            "thrift": ["thrift", "thriftd"],
        }
        for req, dep in self.dependencies.items():
            self._import_dynamic_libs(dep, dldir, dependency_libs.get(req.ref.name, ["*"]))
        if self.dependencies["libcosim"].options.proxyfmu:
            self._import_executables(self.dependencies["proxyfmu"], bindir, ["*"])

        # Generate build system
        CMakeToolchain(self).generate()
        CMakeDeps(self).generate()

    def _import_dynamic_libs(self, dependency, target_dir, patterns):
        if dependency.options.get_safe("shared", False):
            if self.settings.os == "Windows":
                depdirs = dependency.cpp_info.bindirs
            else:
                depdirs = dependency.cpp_info.libdirs
            for depdir in depdirs:
                for pattern in patterns:
                    patternx = pattern+".dll" if self.settings.os == "Windows" else "lib"+pattern+".so*"
                    files = copy(self, patternx, depdir, target_dir, keep_path=False)
                    self._update_rpath(files, "$ORIGIN")

    def _import_executables(self, dependency, target_dir, patterns=["*"]):
        for bindir in dependency.cpp_info.bindirs:
            for pattern in patterns:
                patternx = pattern+".exe" if self.settings.os == "Windows" else pattern
                files = copy(self, patternx, bindir, target_dir, keep_path=False)
                self._update_rpath(files, "$ORIGIN/../lib")

    def _update_rpath(self, files, new_rpath):
        if files and self.settings.os == "Linux":
            with VirtualBuildEnv(self).environment().vars(self).apply():
                self.run("patchelf --set-rpath '" + new_rpath + "' '" + ("' '".join(files)) + "'")
