class Storage:
    """
    A service which provides access to the files in a repository

    """
    def __init__(self, store_type):
        self.store_type = store_type

    def get_file_contents(self, file_path: str):
        """
        Returns the string contents of a file

        """
        pass

    def upload_file(self, local_file_path, dest_file_path):
        """
        Uploads the file to the provided path. If the parent directories to not exist,
        the function will create them as needed.

        """
        pass

    def copy_file(self, src_file_path, dst_file_path):
        """
        Copies the file referenced by src_file_path on the remote repo to the path
        specified by dst_file_path on the remote host.

        """
        pass

    def set_xml_file_contents(self, contents, dest_file_path):
        """
        Sets the string contents of the file represented by the path. If the file does not
        exist, it will create it along with any necessary directories

        """
        pass