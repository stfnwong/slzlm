# Copying this https://stackoverflow.com/questions/10968309/how-to-import-python-module-from-so-file

so_path = "../build/lib.linux-x86_64-cpython-311"
so_name = "slz.cpython-311-x86_64-linux-gnu.so"
so_file = f"{so_path}/{so_name}"

def __bootstrap__():
    global __bootstrap__, __loader__, __file__
    import pkg_resources, imp

    __file__ = pkg_resources.resource_filename(__name__, so_file)
    __loader__ = None; del __bootstrap__, __loader__

    imp.load_dynamic(__name__, __file__)

__bootstrap__()
