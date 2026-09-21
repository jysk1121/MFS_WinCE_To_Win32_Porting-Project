import os


class ManifestV1Builder:
    def __init__(self, signer, input_dir, manifest):
        self.signer = signer
        self.input_dir = input_dir
        self.manifest = manifest

    def _create_file_sig(self, filename):
        """
        Sign the file and return the digest in b64

        """
        return self.signer.sign_file(filename)

    def _add_item(self, filepath, filesize, signature):
        self.manifest.add_item(signature, self.signer.get_fullname(b64=True), filepath, filesize)

    def build_manifest(self):
        """
        Walks the dir/files and calculates the signatures

        """
        for directory, sub_dirs, files in os.walk(self.input_dir):
            for filename in files:

                # Don't include the manifest in the manifest
                if "manifest" in filename:
                    continue

                # Path to the file relative to the execution dir
                full_path = os.path.join(directory, filename)
                # Path to the file relative to the update dir
                item_name = os.path.relpath(full_path, self.input_dir)

                # Get file size
                info = os.stat(full_path)
                filesize = info.st_size

                # Get signature
                sig = self._create_file_sig(full_path)

                # Register the file with the manifest
                self._add_item(item_name, filesize, sig)
