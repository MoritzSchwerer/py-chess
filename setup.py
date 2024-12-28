from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

ext_modules = [
    Pybind11Extension(
        "chess_env",
        ["bindings/bind_perf.cpp", "src/game_state.cpp"],
        include_dirs=["./include"],
        extra_compile_args=["-std=c++2b", "-flto","-march=native", "-mbmi", "-mbmi2", "-msse4.2", "-O3", "-ftree-vectorize"]
    ),
]

setup(
    name="chess_env",
    version="0.1.0",
    author="Moritz Schwerer",
    # author_email="your_email@example.com",
    description="Description of your module",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
