import os
import shutil
from .backend import BackendBase
from ..models import RemotePackage


class FileBackend(BackendBase):
    """
    Downloads the referenced files from the backend

    """

    def __init__(self, base_dir):
        self.base_dir = base_dir

        super(FileBackend, self).__init__()

    def get_index(self):
        """
        Returns the index file from the package repository

        """
        return os.path.join(self.base_dir, "index.xml")

    def install_reference(self, package: RemotePackage, destination: str):
        """
        Installs the version of the reference file to the appropriate location on the local filesystem

        """

        if not os.path.isdir(destination):
            os.makedirs(destination)

        src = os.path.join(self.base_dir, package.path)

        filename = os.path.basename(src)
        dst = os.path.join(destination, filename)

        shutil.copyfile(src, dst)

        return True