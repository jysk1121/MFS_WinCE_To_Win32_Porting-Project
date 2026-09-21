import os
import sys
import shutil
from .storage import Storage


class FileStorage(Storage):
    """
    A service which access a repository using a file path

    """

    def __init__(self, root_path):
        self.root = root_path
        
        super(FileStorage, self).__init__("file")

    def get_file_contents(self, file_path: str):
        """
        Returns the string contents of a file

        """
        # Make the dirs, if neccessary
        fq_path = self._prepare_remote_path(file_path)
        with open(fq_path, 'r') as file:
            return file.read()

    def upload_file(self, local_file_path, dest_file_path):
        """
        Uploads the file to the provided path. If the parent directories to not exist,
        the function will create them as needed.

        """
        # Make the dirs, if neccessary
        fq_path = self._prepare_remote_path(dest_file_path, True)

        # Upload file
        if sys.platform == 'win32':
            os.system('xcopy /Y /Q "%s" "%s" > nul' % (local_file_path, fq_path))
        else:
            shutil.copy2(local_file_path, fq_path)

    def copy_file(self, src_file_path, dst_file_path):
        """
        Copies the file referenced by src_file_path on the remote repo to the path
        specified by dst_file_path on the remote host.

        """
         # Make the dirs, if neccessary
        src_path = self._prepare_remote_path(src_file_path)
        dst_path = self._prepare_remote_path(dst_file_path)

        os.system('xcopy /Y /Q "%s" "%s" > nul' % (src_path, dst_path))

    def set_xml_file_contents(self, contents, dest_file_path):
        """
        Sets the string contents of the file represented by the path. If the file does not
        exist, it will create it along with any necessary directories

        """
        fq_path = self._prepare_remote_path(dest_file_path)
        with open(fq_path, 'wb') as file:
            file.write(contents)

    def _prepare_remote_path(self, relative_path, is_dir=False):
        """
        Ensures that the path exists and creates dirs. if needed. 
        
        @returns the the fully qualified remote path

        """
        if not is_dir:
            relative_dir = os.path.dirname(relative_path)
        else:
            relative_dir = relative_path
        
        fq_dir = os.path.join(self.root, relative_dir)
        os.makedirs(fq_dir, exist_ok=True)

        fq_path = os.path.join(self.root, relative_path)
        return fq_path
