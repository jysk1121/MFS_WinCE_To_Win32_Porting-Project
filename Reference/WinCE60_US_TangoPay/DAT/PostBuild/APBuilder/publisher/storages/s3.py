import os
import boto3
import mimetypes
from botocore.exceptions import ClientError
from .storage import Storage


CONTENT_TYPE_MAP = {
    '.xml': 'application/xml',
    '.json': 'application/json',
    '.zip': 'application/octet-stream',
}

DEFAULT_CONTENT_TYPE = 'application/octet-stream'


class S3Storage(Storage):
    """
    A service which accesses a repository using S3

    """

    def __init__(self, bucket_name, object_prefix):
        self.client = boto3.client('s3')
        self.bucket_name = bucket_name
        self.object_prefix = object_prefix

        super(S3Storage, self).__init__('s3')

    def _convert_to_object_name(self, path: str):
        """
        Fixes the path components in a way that make sense
        for the storage type

        """
        return path.replace("\\", '/')

    def _get_content_type(self, file_name):
        """
        Gets the MIME type for the file (guessing based on the file name)

        """
        content_type = DEFAULT_CONTENT_TYPE

        extension = '.' + file_name.split('.')[-1]

        if extension in CONTENT_TYPE_MAP:
            content_type = CONTENT_TYPE_MAP[extension]

        return content_type

    def get_file_contents(self, file_path: str):
        """
        Returns the string contents of a file

        """
        object_name = os.path.join(self.object_prefix, file_path)
        object_name = self._convert_to_object_name(object_name)
        response = self.client.get_object(
            Key=object_name,
            Bucket=self.bucket_name,
        )
        body = response['Body']
        return body.read().decode('utf-8')

    def upload_file(self, local_file_path, dest_file_path):
        """
        Uploads the file to the provided path. If the parent directories to not exist,
        the function will create them as needed.

        """
        file_name = os.path.basename(local_file_path)
        object_name = os.path.join(self.object_prefix, dest_file_path, file_name)
        object_name = self._convert_to_object_name(object_name)

        self.client.upload_file(
            local_file_path, 
            self.bucket_name, 
            object_name,
            ExtraArgs={
                'ACL': 'public-read',
                'ContentType': self._get_content_type(local_file_path)
            })

    def copy_file(self, src_file_path, dst_file_path):
        """
        Copies the file referenced by src_file_path on the remote repo to the path
        specified by dst_file_path on the remote host.

        """
        src_object_prefix = os.path.join(self.object_prefix, src_file_path)
        src_object_prefix = self._convert_to_object_name(src_object_prefix) + '/'

        dst_object_prefix = os.path.join(self.object_prefix, dst_file_path)
        dst_object_prefix = self._convert_to_object_name(dst_object_prefix) + '/'

        items_remaining = True
        found_token = None
        while True:
            if found_token:
                response = self.client.list_objects_v2(
                    Bucket=self.bucket_name,
                    Prefix=src_object_prefix,
                    Delimiter='/',
                    ContinuationToken=found_token
                )
            else:
                response = self.client.list_objects_v2(
                    Bucket=self.bucket_name,
                    Prefix=src_object_prefix,
                    Delimiter='/'
                )

            for obj in response['Contents']:
                # No need to specify content-type because all meta-data is preserved except
                # the ACL
                self.client.copy_object(
                    CopySource={
                        'Bucket': self.bucket_name,
                        'Key': obj['Key']
                    },
                    Bucket=self.bucket_name,
                    ACL='public-read',
                    Key=dst_object_prefix + obj['Key'].replace(src_object_prefix, '')
                )
            
            if "NextContinuationToken" not in response:
                break

            found_token = response['NextContinuationToken']

    def set_xml_file_contents(self, contents, dest_file_path):
        """
        Sets the string contents of the file represented by the path. If the file does not
        exist, it will create it along with any necessary directories

        """
        object_name = os.path.join(self.object_prefix, dest_file_path)
        object_name = self._convert_to_object_name(object_name)
        self.client.put_object(
            ACL='public-read',
            Body=contents,
            Key=object_name,
            Bucket=self.bucket_name,
            ContentType='application/xml'
        )