import os
from .base_model import BaseModel
from distutils.dir_util import copy_tree

SCREEN_SIZE = '1024_768'

class SE5200(BaseModel):
    """
    A builder for the MX5200SE and MX5300SE

    """

    def __init__(self):
        super(SE5200, self).__init__('5200', [
                ('5200', SCREEN_SIZE, '*'),
                ('2700', '1024_600', 'OP_*'),
                ('2700', '1024_600', 'AP_*')
            ], '5200SE')
