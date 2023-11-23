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
        bindir = os.path.join(self.build_folder, "dist", "bin")
        dldir = (bindir if self.settings.os == "Windows" else
            os.path.join(self.build_folder, "dist", "lib"))
        self._import_dynamic_libs("boost", dldir, [
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
                "boost_thread*",
            ])
        self._import_dynamic_libs("libcosim", dldir)
        self._import_dynamic_libs("openssl", dldir)
        self._import_dynamic_libs("fmilibrary", dldir)
        self._import_dynamic_libs("xerces-c", dldir)
        self._import_dynamic_libs("yaml-cpp", dldir)
        self._import_dynamic_libs("libzip", dldir)
        if self.dependencies["libcosim"].options.proxyfmu:
            self._import_dynamic_libs("proxyfmu", dldir)
            self._import_dynamic_libs("thrift", dldir, ["thrift", "thriftd"])
            self._import_executables("proxyfmu", bindir)
        CMakeToolchain(self).generate()
        CMakeDeps(self).generate()

    def _import_executables(self, dependency_name, target_dir, exenames=["*"]):
        dep = self.dependencies[dependency_name]
        for bindir in dep.cpp_info.bindirs:
            for exename in exenames:
                pattern = exename+".exe" if self.settings.os == "Windows" else exename
                files = copy(self, pattern, bindir, target_dir, keep_path=False)
                self._update_rpath(files, "$ORIGIN/../lib")

    def _import_dynamic_libs(self, dependency_name, target_dir, libnames=["*"]):
        dep = self.dependencies[dependency_name]
        if dep.options.get_safe("shared", False):
            depdirs = dep.cpp_info.bindirs if self.settings.os == "Windows" else dep.cpp_info.libdirs
            for depdir in depdirs:
                for libname in libnames:
                    pattern = libname+".dll" if self.settings.os == "Windows" else "lib"+libname+".so*"
                    files = copy(self, pattern, depdir, target_dir, keep_path=False)
                    self._update_rpath(files, "$ORIGIN")

    def _update_rpath(self, files, new_rpath):
        if files and self.settings.os == "Linux":
            with VirtualBuildEnv(self).environment().vars(self).apply():
                self.run("patchelf --set-rpath '" + new_rpath + "' '" + ("' '".join(files)) + "'")
