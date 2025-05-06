import glob
from pybind11.setup_helpers import Pybind11Extension, build_ext


def build(setup_kwargs):
    ext_modules = [
    Pybind11Extension(
        "_richdem",
        ["src/pywrapper.cpp"] + list(glob.glob("lib/richdem/src/**/*.cpp", recursive=True)),
        include_dirs=["lib/richdem/include"],
        define_macros=[
            ("DOCTEST_CONFIG_DISABLE", None),
            ("_USE_MATH_DEFINES", None),  # Ensure M_PI is available in MSVC
        ],
        language="c++",
    ),
]

    setup_kwargs.update({
        "ext_modules": ext_modules,
        "cmd_class": {"build_ext": build_ext},
        "zip_safe": False,
    })