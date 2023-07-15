# Can I build the module interface?

from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = "0.1.0"

SRC_DIR = "src"

ext_modules = [
    Pybind11Extension(
        "slz",
        sorted(glob(f"{SRC_DIR}/*.cpp"))
        )
]


setup(
    name="slz",
    version=__version__,
    author="Stefan Wong",
    author_email="stfnwong@gmail.com",
    url="https://github.com/stfnwong",
    description="A shitty implementation of Lempel-Ziv compression",
    long_description="",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.9"
)

