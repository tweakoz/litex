import os
from pathlib import Path

this_dir = os.path.dirname(os.path.realpath(__file__))

def wrap(a):
    return Path(str(a))

def root_dir():
    return wrap(this_dir)/".."/".."

def invoking_script():
    wrap(sys.path[0])

def module_dir():
    return wrap(this_dir)/".."

def module_soc_dir():
    return module_dir()/"soc"

def module_build_dir():
    return module_dir()/"build"

def module_xilinx_dir():
    return module_build_dir()/"xilinx"

def module_altera_dir():
    return module_build_dir()/"altera"

def module_lattice_dir():
    return module_build_dir()/"lattice"

def output_build_dir():
    return wrap(os.environ["LITEX_BUILD_DIR"])

def copyfile(_from,_to):
    import shutil
    shutil.copyfile(str(_from),str(_to))
