from ..models import RemotePackage


class BackendBase:
    """
    Backend represents a manner of downloading files from the repository

    """

    def __init__(self):
        pass

    def get_index(self):
        """
        Returns the index file from the package repository

        """
        pass

    def install_reference(self, package: RemotePackage, destination: str):
        """
        Installs the version of the reference file to the appropriate location on the loca filesystem

        """
        pass
