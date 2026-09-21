import os
import shutil
import glob
from .backend import BackendBase
from ..models import RemotePackage

CACHE_DIR = './.cache'


class CacheBackend(BackendBase):
    """
    Downloads the referenced files from the backend, but caches them locally

    """

    def __init__(self, proxy_backend):
        self.proxy_backend = proxy_backend

        super(CacheBackend, self).__init__()

    def get_index(self):
        """
        Returns the index file from the package repository

        """
        return self.proxy_backend.get_index()

    def install_reference(self, package: RemotePackage, destination: str):
        """
        Installs the version of the reference file to the appropriate location on the local filesystem

        """
        package_cache_dir = os.path.join(CACHE_DIR, package.name, package.version)

        # Check to see if the package has been cached
        if not os.path.isdir(package_cache_dir):
            # Download the package, since it hasn't been cached previously
            os.makedirs(package_cache_dir)

            if not self.proxy_backend.install_reference(package, package_cache_dir):
                os.rmdir(package_cache_dir)
                return False

        # Package was cached. Copy the file over
        for name in glob.glob(os.path.join(package_cache_dir, "*")):
            shutil.copy(name, destination)

        return True