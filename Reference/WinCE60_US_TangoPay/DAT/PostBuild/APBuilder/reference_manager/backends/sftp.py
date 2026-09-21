import os
import paramiko

from .backend import BackendBase
from ..models import RemotePackage


LOCAL_INDEX_FILE_PATH = "./.cache/package_index.xml"


class SftpBackend(BackendBase):
    """
    Downloads the reference files from the SFTP site

    """

    def __init__(self, hostname, username, password, base_dir):
        self.hostname = hostname
        self.username = username
        self.password = password
        self.base_dir = base_dir

        self.connection = None
        self.transport = None

        if not os.path.isdir(os.path.dirname(LOCAL_INDEX_FILE_PATH)):
            os.makedirs(os.path.dirname(LOCAL_INDEX_FILE_PATH))

        super(SftpBackend, self).__init__()

    def _get_connection(self):
        if self.connection:
            return self.connection

        self.transport = paramiko.Transport(self.hostname, 22)
        self.transport.connect(username=self.username, password=self.password)

        self.connection = paramiko.SFTPClient.from_transport(self.transport)

        return self.connection

    def _join_path(self, path, *paths):
        path = os.path.join(path, *paths)
        return path.replace("\\", "/")

    def get_index(self):
        """
        Returns the index file from the package repository

        """

        index_src = self._join_path(self.base_dir, "package_index.xml")

        conn = self._get_connection()
        conn.get(index_src, LOCAL_INDEX_FILE_PATH)

        return LOCAL_INDEX_FILE_PATH

    def install_reference(self, package: RemotePackage, destination: str):
        """
        Installs the version of the reference file to the appropriate location on the local filesystem

        """

        if not os.path.isdir(destination):
            os.makedirs(destination)

        src = self._join_path(self.base_dir, package.path)

        filename = os.path.basename(src)
        dst = os.path.join(destination, filename)

        conn = self._get_connection()
        conn.get(src, dst)

        return True