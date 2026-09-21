import semver
import os

from .backends import BackendBase
from .models import Reference, RemotePackage
from .serialization import references_from_file, packages_from_file

def version_is_greater(version1, version2):
    """
    @returns True when version2 > version1

    """

    return semver.compare(version1, version2) == -1


class Manage:
    """
    A utility for downloading/synchronizing reference files

    """

    def __init__(self, backend: BackendBase, reference_file: str, customer: str, territory: str):
        self.backend = backend
        self.reference_file = reference_file
        self.customer = customer
        self.territory = territory

    def _get_references(self):
        """
        Gets a list of references as defined by the reference file

        """

        return references_from_file(self.reference_file, self.customer, self.territory)

    def _is_newer_version(self, index: [RemotePackage], name: str, version: str):
        """
        Attempts to determine if a newer version of a package is available

        @returns a tuple of the whether a newer package was found and the new version number

        """
        highest_version = None

        for package in index:
            if package.name == name:
                if highest_version == None or version_is_greater(highest_version, package.version):
                    highest_version = package.version

        return (version != highest_version, highest_version)

    def _find_matching_remote_package(self, index: [RemotePackage], name: str, version: str):
        """
        Finds the matching package reference in the remote index

        """

        for package in index:
            if package.name == name and package.version == version:
                return package

        return None

    def _download_files(self, index: [RemotePackage], references: [Reference], output_dir: str):
        """
        Downloads the files from the backend

        """

        for reference in references:
            name = reference.name
            version = reference.version

            package = self._find_matching_remote_package(index, name, version)

            if package is None:
                # The package wasn't found
                print(f"Warning: {reference.name}@{reference.version} was not found!")
                continue

            (newer, new_version) = self._is_newer_version(index, name, version)
            if newer:
                # Warn that a newer version is available
                print(f"Warning: An upgraded version of {name} is available: {version} -> {new_version}")

            # Create dest dir
            dst = os.path.join(output_dir, reference.destination)
            if not os.path.isdir(dst):
                os.makedirs(dst)

            # Install it!
            self.backend.install_reference(package, dst)

    def synchronize_references(self, output_dir: str):
        # 1. Download the index from the backend
        index_file = self.backend.get_index()
        index = packages_from_file(index_file)
        references = self._get_references()

        self._download_files(index, references, output_dir)
