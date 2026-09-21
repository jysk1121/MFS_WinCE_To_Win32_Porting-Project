import os
from .base_model import BaseModel
from distutils.dir_util import copy_tree

SCREEN_SIZE = '640_480'

class SE1500(BaseModel):
    """
    A builder for the 1500SE

    """

    def __init__(self):
        super(SE1500, self).__init__('1500', [
                ('1500', SCREEN_SIZE, '*')
            ], '1500SE')
